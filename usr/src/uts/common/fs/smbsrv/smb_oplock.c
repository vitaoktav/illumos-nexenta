/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/*
 * Copyright (c) 2007, 2010, Oracle and/or its affiliates. All rights reserved.
 * Copyright 2016 Nexenta Systems, Inc.  All rights reserved.
 */

/*
 * smb_oplock_wait / smb_oplock_broadcast
 * When an oplock is being acquired, we must ensure that the acquisition
 * response is submitted to the network stack before any other operation
 * is permitted on the oplock.
 * In smb_oplock_acquire, oplock.ol_xthread is set to point to the worker
 * thread processing the command that is granting the oplock.
 * Other threads accessing the oplock will be suspended in smb_oplock_wait().
 * They will be awakened when the worker thread referenced in 'ol_xthread'
 * calls smb_oplock_broadcast().
 *
 * The purpose of this mechanism is to prevent another thread from
 * triggering an oplock break before the response conveying the grant
 * has been sent.
 */

#include <smbsrv/smb_kproto.h>
#include <sys/nbmlock.h>

#define	SMB_OPLOCK_IS_EXCLUSIVE(level)		\
	(((level) == SMB_OPLOCK_EXCLUSIVE) ||	\
	((level) == SMB_OPLOCK_BATCH))

static int smb_oplock_install_fem(smb_node_t *);
static void smb_oplock_uninstall_fem(smb_node_t *);

static void smb_oplock_wait(smb_node_t *);
static void smb_oplock_wait_ack(smb_node_t *, uint32_t);
static void smb_oplock_timedout(smb_node_t *);

static smb_oplock_grant_t *smb_oplock_set_grant(smb_ofile_t *, uint8_t);
void smb_oplock_clear_grant(smb_oplock_grant_t *);
static int smb_oplock_insert_grant(smb_node_t *, smb_oplock_grant_t *);
static void smb_oplock_remove_grant(smb_node_t *, smb_oplock_grant_t *);
static smb_oplock_grant_t *smb_oplock_exclusive_grant(list_t *);
static smb_oplock_grant_t *smb_oplock_get_grant(smb_oplock_t *, smb_ofile_t *);

static void smb_oplock_sched_async_break(smb_oplock_grant_t *, uint8_t);
static void smb_oplock_async_close(void *);
static void smb_oplock_async_sendbreak(void *);
static int smb_oplock_send_break(smb_request_t *, uint8_t);
static void smb_oplock_break_levelII_locked(smb_node_t *);

/*
 * smb_oplock_install_fem
 * Install fem monitor for cross protocol oplock breaking.
 */
static int
smb_oplock_install_fem(smb_node_t *node)
{
	ASSERT(MUTEX_HELD(&node->n_oplock.ol_mutex));

	if (node->n_oplock.ol_fem == B_FALSE) {
		if (smb_fem_oplock_install(node) != 0) {
			cmn_err(CE_NOTE, "No oplock granted: "
			    "failed to install fem monitor %s",
			    node->vp->v_path);
			return (-1);
		}
		node->n_oplock.ol_fem = B_TRUE;
	}
	return (0);
}

/*
 * smb_oplock_uninstall_fem
 * Uninstall fem monitor for cross protocol oplock breaking.
 */
static void
smb_oplock_uninstall_fem(smb_node_t *node)
{
	ASSERT(MUTEX_HELD(&node->n_oplock.ol_mutex));

	if (node->n_oplock.ol_fem) {
		smb_fem_oplock_uninstall(node);
		node->n_oplock.ol_fem = B_FALSE;
	}
}

/*
 * This provides a way to fully disable oplocks, i.e. for testing.
 * You _really_ do _not_ want to turn this off, because if you do,
 * the clients send you very small read requests, and a _lot_ more
 * of them.  The skc_oplock_enable parameter can be used to enable
 * or disable exclusive oplocks.  Disabling that can be helpful
 * when there are clients not responding to oplock breaks.
 */
int smb_oplocks_enabled = 1;

/*
 * smb_oplock_acquire
 *
 * Attempt to acquire an oplock. Clients will request EXCLUSIVE or BATCH,
 * but might only be granted LEVEL_II or NONE.
 *
 * If oplocks are not supported on the tree, or node, grant NONE.
 * If nobody else has the file open, grant the requested level.
 * If any of the following are true, grant NONE:
 * - there is an exclusive oplock on the node
 * - op->op_oplock_levelII is B_FALSE (LEVEL_II not supported by open cmd.
 * - LEVEL_II oplocks are not supported for the session
 * - a BATCH oplock is requested on a named stream
 * - there are any range locks on the node (SMB writers)
 * Otherwise, grant LEVEL_II.
 *
 * ol->ol_xthread is set to the current thread to lock the oplock against
 * other operations until the acquire response is on the wire. When the
 * acquire response is on the wire, smb_oplock_broadcast() is called to
 * reset ol->ol_xthread and wake any waiting threads.
 */
void
smb_oplock_acquire(smb_request_t *sr, smb_node_t *node, smb_ofile_t *ofile)
{
	smb_oplock_t		*ol;
	smb_oplock_grant_t	*og;
	list_t			*grants;
	smb_arg_open_t		*op;
	smb_tree_t		*tree;
	smb_session_t		*session;

	SMB_NODE_VALID(node);
	SMB_OFILE_VALID(ofile);

	ASSERT(node == SMB_OFILE_GET_NODE(ofile));
	ASSERT(RW_LOCK_HELD(&node->n_lock));

	op = &sr->sr_open;

	if (smb_oplocks_enabled == 0 ||
	    (op->op_oplock_level == SMB_OPLOCK_NONE) ||
	    ((op->op_oplock_level == SMB_OPLOCK_BATCH) &&
	    SMB_IS_STREAM(node))) {
		op->op_oplock_level = SMB_OPLOCK_NONE;
		return;
	}

	ol = &node->n_oplock;
	grants = &ol->ol_grants;

	mutex_enter(&ol->ol_mutex);
	smb_oplock_wait(node);
	tree = SMB_OFILE_GET_TREE(ofile);
	session = SMB_OFILE_GET_SESSION(ofile);

	/*
	 * Even if there are no other opens, we might want to
	 * grant only a Level II (shared) oplock so we avoid
	 * ever granting exclusive oplocks.
	 *
	 * Borrowing the SMB_TREE_OPLOCKS flag to enable/disable
	 * exclusive oplocks (for now).  See skc_oplock_enable,
	 * which can now be taken as "exclusive oplock enable".
	 * Should rename this parameter, and/or implement a new
	 * multi-valued parameter for oplock enables.
	 */
	if ((node->n_open_count > 1) ||
	    (node->n_opening_count > 1) ||
	    !smb_tree_has_feature(tree, SMB_TREE_OPLOCKS) ||
	    smb_vop_other_opens(node->vp, ofile->f_mode)) {
		/*
		 * There are other opens.
		 */
		if ((!op->op_oplock_levelII) ||
		    (!smb_session_levelII_oplocks(session)) ||
		    (smb_oplock_exclusive_grant(grants) != NULL) ||
		    (smb_lock_range_access(sr, node, 0, 0, B_FALSE))) {
			/*
			 * LevelII (shared) oplock not allowed,
			 * so reply with "none".
			 */
			op->op_oplock_level = SMB_OPLOCK_NONE;
			mutex_exit(&ol->ol_mutex);
			return;
		}

		op->op_oplock_level = SMB_OPLOCK_LEVEL_II;
	}

	og = smb_oplock_set_grant(ofile, op->op_oplock_level);
	if (smb_oplock_insert_grant(node, og) != 0) {
		smb_oplock_clear_grant(og);
		op->op_oplock_level = SMB_OPLOCK_NONE;
		mutex_exit(&ol->ol_mutex);
		return;
	}

	ol->ol_xthread = curthread;
	mutex_exit(&ol->ol_mutex);
}

/*
 * smb_oplock_break
 *
 * Break granted oplocks according to the following rules:
 *
 * If there's an exclusive oplock granted on the node
 *  - if the BREAK_BATCH flags is specified and the oplock is not
 *    a batch oplock, no break is required.
 *  - if the session doesn't support LEVEL II oplocks, and 'brk' is
 *    BREAK_TO_LEVEL_II, do a BREAK_TO_NONE.
 *  - if the oplock is already breaking update the break level (if
 *    the requested break is to a lesser level), otherwise send an
 *    oplock break.
 *    Wait for acknowledgement of the break (unless NOWAIT flag is set)
 *
 * Otherwise:
 * If there are level II oplocks granted on the node, and the flags
 * indicate that they should be broken (BREAK_TO_NONE specified,
 * BREAK_EXCLUSIVE, BREAK_BATCH not specified) queue the levelII
 * break request for asynchronous processing.
 *
 * Returns:
 *       0 - oplock broken (or no break required)
 *  EAGAIN - oplock break request sent and would block
 *           awaiting the reponse but NOWAIT was specified
 *
 * NB: sr == NULL when called by FEM framework.
 */
int
smb_oplock_break(smb_request_t *sr, smb_node_t *node, uint32_t flags)
{
	smb_oplock_t		*ol;
	smb_oplock_grant_t	*og;
	smb_ofile_t		*ofile;
	list_t			*grants;
	uint32_t		timeout;
	uint8_t			brk;

	SMB_NODE_VALID(node);
	ol = &node->n_oplock;
	grants = &ol->ol_grants;

	mutex_enter(&ol->ol_mutex);
	smb_oplock_wait(node);

	og = list_head(grants);
	if (og == NULL) {
		mutex_exit(&ol->ol_mutex);
		return (0);
	}

	SMB_OPLOCK_GRANT_VALID(og);
	ofile = og->og_ofile;	/* containing struct */

	/* break levelII oplocks */
	if (og->og_level == SMB_OPLOCK_LEVEL_II) {
		mutex_exit(&ol->ol_mutex);

		if ((flags & SMB_OPLOCK_BREAK_TO_NONE) &&
		    !(flags & SMB_OPLOCK_BREAK_EXCLUSIVE) &&
		    !(flags & SMB_OPLOCK_BREAK_BATCH))  {
			smb_oplock_break_levelII(node);
		}
		return (0);
	}

	/* break exclusive oplock */
	if ((flags & SMB_OPLOCK_BREAK_BATCH) &&
	    (og->og_level != SMB_OPLOCK_BATCH)) {
		mutex_exit(&ol->ol_mutex);
		return (0);
	}

	if ((flags & SMB_OPLOCK_BREAK_TO_LEVEL_II) &&
	    smb_session_levelII_oplocks(ofile->f_session)) {
		brk = SMB_OPLOCK_BREAK_TO_LEVEL_II;
	} else {
		brk = SMB_OPLOCK_BREAK_TO_NONE;
	}

	switch (ol->ol_break) {
	case SMB_OPLOCK_NO_BREAK:
		ol->ol_break = brk;
		smb_oplock_sched_async_break(og, brk);
		break;
	case SMB_OPLOCK_BREAK_TO_LEVEL_II:
		if (brk == SMB_OPLOCK_BREAK_TO_NONE)
			ol->ol_break = SMB_OPLOCK_BREAK_TO_NONE;
		break;
	case SMB_OPLOCK_BREAK_TO_NONE:
	default:
		break;
	}

	if (flags & SMB_OPLOCK_BREAK_NOWAIT) {
		mutex_exit(&ol->ol_mutex);
		return (EAGAIN);
	}

	if (sr != NULL && sr->uid_user == ofile->f_user) {
		timeout = smb_oplock_min_timeout;
	} else {
		timeout = smb_oplock_timeout;
	}

	mutex_exit(&ol->ol_mutex);
	smb_oplock_wait_ack(node, timeout);
	return (0);
}

/*
 * smb_oplock_break_levelII
 *
 * This is called after a file is modified in some way.  If there are
 * LevelII (shared) oplocks, break those to none.  If there is an
 * exclusive oplock, there can be no LevelII oplocks, so do nothing.
 *
 * LevelII (shared) oplock breaks are processed asynchronously.
 * Unlike exclusive oplock breaks, the thread initiating the break
 * is NOT blocked while the request is processed.
 *
 * There may be a thread with exclusive rights to oplock state for
 * this node (via ol_xthread in smb_oplock_wait) and if so, we must
 * avoid breaking oplocks until that's out of the way.  However, we
 * really don't want to block here, so when ol_xthread is set, we'll
 * just mark that a "break level II to none" is pending, and let the
 * exclusive thread do this work when it's done being exclusive.
 */
void
smb_oplock_break_levelII(smb_node_t *node)
{
	smb_oplock_t		*ol;

	ol = &node->n_oplock;
	mutex_enter(&ol->ol_mutex);

	/* Instead of: smb_oplock_wait() ... */
	if (ol->ol_xthread != NULL) {
		/* Defer the call to smb_oplock_broadcast(). */
		ol->ol_brk_pending = SMB_OPLOCK_BREAK_TO_NONE;
	} else {
		/* Equivalent of smb_oplock_wait() done. */
		smb_oplock_break_levelII_locked(node);
	}

	mutex_exit(&ol->ol_mutex);
}

/*
 * smb_oplock_break_levelII_locked
 * Internal helper for smb_oplock_break_levelII()
 *
 * Called with the oplock mutex already held, and _after_
 * (the equivalent of) an smb_oplock_wait().
 */
static void
smb_oplock_break_levelII_locked(smb_node_t *node)
{
	smb_oplock_t		*ol;
	smb_oplock_grant_t	*og;
	list_t			*grants;

	ol = &node->n_oplock;
	grants = &ol->ol_grants;

	ASSERT(MUTEX_HELD(&ol->ol_mutex));
	ASSERT(ol->ol_xthread == NULL);

	while ((og = list_head(grants)) != NULL) {
		SMB_OPLOCK_GRANT_VALID(og);

		/*
		 * If there's an exclusive oplock, there are
		 * no LevelII oplocks, so do nothing.
		 */
		if (SMB_OPLOCK_IS_EXCLUSIVE(og->og_level))
			break;

		smb_oplock_sched_async_break(og, SMB_OPLOCK_BREAK_TO_NONE);
		/*
		 * If oplock break had to do a "local" ack, the
		 * oplock grant will have been already removed.
		 */
		if (og->og_magic == SMB_OPLOCK_GRANT_MAGIC) {
			smb_oplock_remove_grant(node, og);
			smb_oplock_clear_grant(og);
		}
	}
}

/*
 * Schedule an asynchronous call to smb_oplock_send_break.
 * Try hard to avoid waiting for any oplock work here, or
 * callers coming in from FEM etc. may suffer delays.
 *
 * The caller holds the oplock mutex, and will
 * signal ol_cv after we return.
 */
static void
smb_oplock_sched_async_break(smb_oplock_grant_t *og, uint8_t brk)
{
	smb_ofile_t	*ofile;
	boolean_t	do_close = B_FALSE;
	boolean_t	do_send = B_FALSE;
	boolean_t	local_break = B_FALSE;

	ofile = og->og_ofile;	/* containing struct */
	SMB_OFILE_VALID(ofile);

	/*
	 * Make sure we can get a hold on the ofile.  Do this
	 * inline with special logic to transition from state
	 * ORPHANED to state EXPIRED (when we'll close).
	 * This hold is released at the completion of the
	 * SMB request we schedule via taskq_dispatch.
	 */
	mutex_enter(&ofile->f_mutex);

	switch (ofile->f_state) {

	case SMB_OFILE_STATE_OPEN:
		/*
		 * inline smb_ofile_hold(), released in
		 * smb_oplock_async_sendbreak
		 */
		ofile->f_refcnt++;
		do_send = B_TRUE;
		break;

	case SMB_OFILE_STATE_RECONNECT:
		/*
		 * The ofile is about to enter state OPEN by
		 * means of a RECONNECT request; break the oplock
		 * now so that the client will see the new level
		 * in the response.
		 */
		local_break = B_TRUE;
		break;

	case SMB_OFILE_STATE_ORPHANED:
		/*
		 * There is no client connection right now so just
		 * break the oplock or close the handle.  The client
		 * will find out when they reconnect this handle.
		 */
		local_break = B_TRUE;
		switch (ofile->dh_vers) {
		case SMB2_RESILIENT:
			break;
		case SMB2_DURABLE_V2:
			if (ofile->dh_persist == B_FALSE)
				do_close = B_TRUE;
			break;
		case SMB2_DURABLE_V1:
			do_close = B_TRUE;
			break;
		case SMB2_NOT_DURABLE:
			break;
		}
		if (do_close) {
			ofile->f_state = SMB_OFILE_STATE_EXPIRED;
			/*
			 * inline smb_ofile_hold(), release in
			 * smb_oplock_async_close
			 */
			ofile->f_refcnt++;
		}
		break;

	default:
		/* Closing (etc).  No oplock work to do. */
		break;
	}
	mutex_exit(&ofile->f_mutex);

	if (do_send) {
		smb_request_t *sr;

		sr = smb_request_alloc(ofile->f_session, 0);
		if (sr == NULL) {
			/*
			 * The session is in a state that disallows
			 * new requests, which implies we can't send.
			 * Handle the same as local_break.
			 */
			local_break = B_TRUE;
			smb_ofile_release(ofile);
		} else {
			sr->sr_state = SMB_REQ_STATE_SUBMITTED;
			sr->user_cr = zone_kcred();
			sr->fid_ofile = ofile;
			/* Leave tid_tree, uid_user NULL. */
			sr->arg.olbrk = *og; /* struct copy */
			sr->arg.olbrk.og_breaking = brk;

			(void) taskq_dispatch(
			    sr->sr_server->sv_worker_pool,
			    smb_oplock_async_sendbreak, sr, TQ_SLEEP);
		}
	}

	if (do_close) {
		/*
		 * The actual close may need to do some I/O,
		 * so let's use a taskq job for the close.
		 * (See "avoid waiting" above.)
		 * Caller may wait in oplock_wait_ack until
		 * close calls smb_oplock_release.
		 */
		(void) taskq_dispatch(
		    ofile->f_server->sv_worker_pool,
		    smb_oplock_async_close, ofile, TQ_SLEEP);
	} else if (local_break) {
		smb_node_t	*node = ofile->f_node;
		smb_oplock_t	*ol = &node->n_oplock;

		/*
		 * The ofile was in state ORPHANED or RECONNECT.
		 * We do the update locally that the client would have
		 * done if they had received our oplock break message.
		 * Similar to smb_oplock_ack.
		 */
		switch (brk) {
		case SMB_OPLOCK_BREAK_TO_NONE:
			og->og_level = SMB_OPLOCK_NONE;
			break;
		case SMB_OPLOCK_BREAK_TO_LEVEL_II:
			og->og_level = SMB_OPLOCK_LEVEL_II;
			break;
		default:
			SMB_PANIC();
		}

		if (og->og_level == SMB_OPLOCK_NONE) {
			smb_oplock_remove_grant(node, og);
			smb_oplock_clear_grant(og);
		}

		ol->ol_break = SMB_OPLOCK_NO_BREAK;
	}
}

static void
smb_oplock_async_close(void *arg)
{
	smb_ofile_t *ofile = arg;

	smb_ofile_close(ofile, 0);
	smb_ofile_release(ofile);
}

/*
 * smb_oplock_async_sendbreak
 *
 * Called via the taskq to handle an asynchronous oplock break.
 * We have a hold on the ofile, which will be released in
 * smb_request_free (via sr->fid_ofile)
 *
 * Note we have: sr->uid_user == NULL, sr->tid_tree == NULL.
 * Nothing called here needs those.
 */
static void
smb_oplock_async_sendbreak(void *arg)
{
	smb_request_t *sr = arg;
	smb_ofile_t *ofile = sr->fid_ofile;
	smb_oplock_grant_t *og = &sr->arg.olbrk;
	int rc;

	SMB_REQ_VALID(sr);
	SMB_OPLOCK_GRANT_VALID(og);

	mutex_enter(&sr->sr_mutex);
	sr->sr_worker = curthread;
	sr->sr_state = SMB_REQ_STATE_ACTIVE;
	mutex_exit(&sr->sr_mutex);

	/*
	 * This is where we actually do the deferred send
	 * requested by smb_oplock_sched_async_break().
	 */
	rc = smb_oplock_send_break(sr, og->og_breaking);

	/*
	 * If we were unable to send the oplock break request,
	 * we need to handle it locally (simultate the ACK).
	 */
	if (rc != 0)
		smb_oplock_ack(ofile->f_node, ofile, og->og_breaking);

	sr->sr_state = SMB_REQ_STATE_COMPLETED;
	smb_request_free(sr);
}

/*
 * smb_oplock_send_break
 *
 * Send an oplock break request to the client,
 * recalling some cache delegation.
 */
static int
smb_oplock_send_break(smb_request_t *sr, uint8_t brk)
{
	smb_session_t	*session = sr->session;
	mbuf_chain_t	*mbc = &sr->reply;
	int rc;

	SMB_SESSION_VALID(session);

	/*
	 * Build the break message in sr->reply and then send it.
	 * The mbc is free'd later, in smb_request_free().
	 */
	mbc->max_bytes = MLEN;
	if (session->dialect >= SMB_VERS_2_BASE) {
		smb2_oplock_break_notification(sr, brk);
	} else {
		smb1_oplock_break_notification(sr, brk);
	}

	rc = smb_session_send(session, 0, mbc);
	return (rc);
}

/*
 * smb_oplock_wait_ack
 *
 * Timed wait for an oplock break acknowledgement (or oplock release).
 */
static void
smb_oplock_wait_ack(smb_node_t *node, uint32_t timeout)
{
	smb_oplock_t	*ol;
	clock_t		time;

	ol = &node->n_oplock;
	mutex_enter(&ol->ol_mutex);
	time = MSEC_TO_TICK(timeout) + ddi_get_lbolt();

	while (ol->ol_break != SMB_OPLOCK_NO_BREAK) {
		if (cv_timedwait(&ol->ol_cv, &ol->ol_mutex, time) < 0) {
			smb_oplock_timedout(node);
			cv_broadcast(&ol->ol_cv);
			break;
		}
	}
	mutex_exit(&ol->ol_mutex);
}

/*
 * smb_oplock_timedout
 *
 * An oplock break has not been acknowledged within timeout
 * 'smb_oplock_timeout'.
 * Set oplock grant to the desired break level.
 */
static void
smb_oplock_timedout(smb_node_t *node)
{
	smb_oplock_t		*ol;
	smb_oplock_grant_t	*og;
	list_t			*grants;

	ol = &node->n_oplock;
	grants = &ol->ol_grants;

	ASSERT(MUTEX_HELD(&ol->ol_mutex));

	og = smb_oplock_exclusive_grant(grants);
	if (og) {
		switch (ol->ol_break) {
		case SMB_OPLOCK_BREAK_TO_NONE:
			og->og_level = SMB_OPLOCK_NONE;
			smb_oplock_remove_grant(node, og);
			smb_oplock_clear_grant(og);
			break;
		case SMB_OPLOCK_BREAK_TO_LEVEL_II:
			og->og_level = SMB_OPLOCK_LEVEL_II;
			break;
		default:
			SMB_PANIC();
		}
	}
	ol->ol_break = SMB_OPLOCK_NO_BREAK;
}

/*
 * smb_oplock_release
 *
 * Release the oplock granted on ofile 'of'.
 * Wake any threads waiting for an oplock break acknowledgement for
 * this oplock.
 * This is called when the ofile is being closed.
 */
void
smb_oplock_release(smb_node_t *node, smb_ofile_t *of)
{
	smb_oplock_t		*ol;
	smb_oplock_grant_t	*og;

	ol = &node->n_oplock;
	mutex_enter(&ol->ol_mutex);
	smb_oplock_wait(node);

	og = smb_oplock_get_grant(ol, of);
	if (og) {
		smb_oplock_remove_grant(node, og);
		smb_oplock_clear_grant(og);

		if (ol->ol_break != SMB_OPLOCK_NO_BREAK) {
			ol->ol_break = SMB_OPLOCK_NO_BREAK;
			cv_broadcast(&ol->ol_cv);
		}
	}

	mutex_exit(&ol->ol_mutex);
}

/*
 * smb_oplock_ack
 *
 * Process oplock acknowledgement received for ofile 'of'.
 * - oplock.ol_break is the break level that was requested.
 * - brk is the break level being acknowledged by the client.
 *
 * Update the oplock grant level to the lesser of ol_break and brk.
 * If the grant is now SMB_OPLOCK_NONE, remove the grant from the
 * oplock's grant list and delete it.
 * If the requested break level (ol_break) was NONE and the brk is
 * LEVEL_II, send another oplock break (NONE). Do not wait for an
 * acknowledgement.
 * Wake any threads waiting for the oplock break acknowledgement.
 */
void
smb_oplock_ack(smb_node_t *node, smb_ofile_t *of, uint8_t brk)
{
	smb_oplock_t		*ol;
	smb_oplock_grant_t	*og;

	ol = &node->n_oplock;
	mutex_enter(&ol->ol_mutex);
	smb_oplock_wait(node);

	if ((ol->ol_break == SMB_OPLOCK_NO_BREAK) ||
	    ((og = smb_oplock_get_grant(ol, of)) == NULL)) {
		mutex_exit(&ol->ol_mutex);
		return;
	}

	switch (brk) {
	case SMB_OPLOCK_BREAK_TO_NONE:
		og->og_level = SMB_OPLOCK_NONE;
		break;
	case SMB_OPLOCK_BREAK_TO_LEVEL_II:
		if (ol->ol_break == SMB_OPLOCK_BREAK_TO_LEVEL_II) {
			og->og_level = SMB_OPLOCK_LEVEL_II;
		} else {
			/* SMB_OPLOCK_BREAK_TO_NONE */
			og->og_level = SMB_OPLOCK_NONE;
			smb_oplock_sched_async_break(og,
			    SMB_OPLOCK_BREAK_TO_NONE);
		}
		break;
	default:
		SMB_PANIC();
	}

	if (og->og_level == SMB_OPLOCK_NONE) {
		smb_oplock_remove_grant(node, og);
		smb_oplock_clear_grant(og);
	}

	ol->ol_break = SMB_OPLOCK_NO_BREAK;
	cv_broadcast(&ol->ol_cv);

	mutex_exit(&ol->ol_mutex);
}

/*
 * smb_oplock_broadcast
 *
 * Called when an open with oplock request completes.
 *
 * ol->ol_xthread identifies the thread that was performing an oplock
 * acquire. Other threads may be blocked awaiting completion of the
 * acquire.
 * If the calling thread is ol_xthread, wake any waiting threads.
 */
void
smb_oplock_broadcast(smb_node_t *node)
{
	smb_oplock_t	*ol;

	SMB_NODE_VALID(node);
	ol = &node->n_oplock;

	mutex_enter(&ol->ol_mutex);
	if ((ol->ol_xthread != NULL) && (ol->ol_xthread == curthread)) {
		ol->ol_xthread = NULL;
		if (ol->ol_brk_pending) {
			ol->ol_brk_pending = 0;
			smb_oplock_break_levelII_locked(node);
		}
		cv_broadcast(&ol->ol_cv);
	}
	mutex_exit(&ol->ol_mutex);
}

/*
 * smb_oplock_wait
 *
 * Wait for the completion of an oplock acquire.
 * If ol_xthread is not NULL and doesn't contain the pointer to the
 * context of the calling thread, the caller will sleep until the
 * ol_xthread is reset to NULL (via smb_oplock_broadcast()).
 */
static void
smb_oplock_wait(smb_node_t *node)
{
	smb_oplock_t	*ol;

	ol = &node->n_oplock;
	ASSERT(MUTEX_HELD(&ol->ol_mutex));

	if ((ol->ol_xthread != NULL) && (ol->ol_xthread != curthread)) {
		while (ol->ol_xthread != NULL)
			cv_wait(&ol->ol_cv, &ol->ol_mutex);
	}
}

/*
 * smb_oplock_set_grant
 */
static smb_oplock_grant_t *
smb_oplock_set_grant(smb_ofile_t *of, uint8_t level)
{
	smb_oplock_grant_t	*og;

	og = &of->f_oplock_grant;

	og->og_magic = SMB_OPLOCK_GRANT_MAGIC;
	og->og_breaking = 0;
	og->og_level = level;
	og->og_ofile = of;

	return (og);
}

/*
 * smb_oplock_clear_grant
 */
void
smb_oplock_clear_grant(smb_oplock_grant_t *og)
{
	bzero(og, sizeof (smb_oplock_grant_t));
}

/*
 * smb_oplock_insert_grant
 *
 * If there are no grants in the oplock's list install the fem
 * monitor.
 * Insert the grant into the list and increment the grant count.
 */
static int
smb_oplock_insert_grant(smb_node_t *node, smb_oplock_grant_t *og)
{
	smb_oplock_t *ol = &node->n_oplock;

	ASSERT(MUTEX_HELD(&ol->ol_mutex));

	if (ol->ol_count == 0) {
		if (smb_oplock_install_fem(node) != 0)
			return (-1);
	}

	list_insert_tail(&ol->ol_grants, og);
	++ol->ol_count;
	return (0);
}

/*
 * smb_oplock_remove_grant
 *
 * Remove the oplock grant from the list, decrement the grant count
 * and, if there are no other grants in the list, uninstall the fem
 * monitor.
 */
static void
smb_oplock_remove_grant(smb_node_t *node, smb_oplock_grant_t *og)
{
	smb_oplock_t *ol = &node->n_oplock;

	ASSERT(MUTEX_HELD(&ol->ol_mutex));
	ASSERT(ol->ol_count > 0);

	list_remove(&ol->ol_grants, og);
	if (--ol->ol_count == 0)
		smb_oplock_uninstall_fem(node);
}

/*
 * smb_oplock_exclusive_grant
 *
 * If an exclusive (EXCLUSIVE or BATCH) oplock grant exists,
 * return it. Otherwise return NULL.
 */
static smb_oplock_grant_t *
smb_oplock_exclusive_grant(list_t *grants)
{
	smb_oplock_grant_t	*og;

	og = list_head(grants);
	if (og) {
		SMB_OPLOCK_GRANT_VALID(og);
		if (SMB_OPLOCK_IS_EXCLUSIVE(og->og_level))
			return (og);
	}
	return (NULL);
}

/*
 * smb_oplock_get_grant
 *
 * Find oplock grant corresponding to the specified ofile.
 */
static smb_oplock_grant_t *
smb_oplock_get_grant(smb_oplock_t *ol, smb_ofile_t *ofile)
{
	ASSERT(MUTEX_HELD(&ol->ol_mutex));

	if (SMB_OFILE_OPLOCK_GRANTED(ofile))
		return (&ofile->f_oplock_grant);
	else
		return (NULL);
}
