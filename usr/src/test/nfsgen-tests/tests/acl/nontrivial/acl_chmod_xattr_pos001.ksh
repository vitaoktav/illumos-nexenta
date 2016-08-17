#! /usr/bin/ksh -p
#
# CDDL HEADER START
#
# The contents of this file are subject to the terms of the
# Common Development and Distribution License (the "License").
# You may not use this file except in compliance with the License.
#
# You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
# or http://www.opensolaris.org/os/licensing.
# See the License for the specific language governing permissions
# and limitations under the License.
#
# When distributing Covered Code, include this CDDL HEADER in each
# file and include the License file at usr/src/OPENSOLARIS.LICENSE.
# If applicable, add the following below this CDDL HEADER, with the
# fields enclosed by brackets "[]" replaced with your own identifying
# information: Portions Copyright [yyyy] [name of copyright owner]
#
# CDDL HEADER END
#

#
# Copyright 2009 Sun Microsystems, Inc.  All rights reserved.
# Use is subject to license terms.
#

. $STF_SUITE/tests/acl/acl_common.kshlib

#################################################################################
#
# __stc_assertion_start
#
# ID: acl_chmod_xattr_pos001
#
# DESCRIPTION:
#	Verify that the read_xattr/write_xattr for 
#	owner/group/everyone are correct.
#
# STRATEGY:
# 1. Create file and  directory in nfs filesystem
# 2. Set special read_xattr ACE to the file and directory
# 3. Try to list the extended attributes of the file and directory
# 4. Set special write_xattr ACE to the file and directory
# 5. Try to add new extended attributes to the file and directory
# 6. Verify above operation is successful.
#
# TESTABILITY: explicit
#
# TEST_AUTOMATION_LEVEL: automated
#
# __stc_assertion_end
#
################################################################################

[[ :$NFSGEN_DEBUG: = *:${NAME}:* || :${NFSGEN_DEBUG}: = *:all:* ]] \
	&& set -x

function case_cleanup
{
	[[ :$NFSGEN_DEBUG: = *:${NAME}:* || :${NFSGEN_DEBUG}: = *:all:* ]] \
		&& set -x

	[[ $1 != $STF_PASS ]] && [[ -f $STF_TMPDIR/$NAME.$$ ]] \
		&& cat $STF_TMPDIR/$NAME.$$
	rm -rf $STF_TMPDIR/$NAME.$$.*

	cd $cwd

	cleanup_test_files $TESTDIR/basedir

	if [[ -e $TESTDIR/$ARCHIVEFILE ]]; then
		RUN_CHECK $RM -f $TESTDIR/$ARCHIVEFILE || cleanup $STF_FAIL
	fi

	cleanup $1
}

#	owner@	group	group_users		other_users
set -A users \
	"root"	"root"	"$ACL_ADMIN" 	"$ACL_OTHER1" \
	"$ACL_STAFF1"	"$ACL_STAFF_GROUP"	"$ACL_STAFF2" 	"$ACL_OTHER1"

set -A a_access \
	"read_xattr:allow" \
	"read_xattr:deny" \
	"write_xattr:allow" \
	"write_xattr:deny"

set -A a_flag "owner@" "group@" "everyone@"

MYTESTFILE=$STF_SUITE/STF.INFO

echo "ASSERTION: Verify that the permission of read_xattr/write_xattr for " \
	"owner/group/everyone are correct."

function operate_node #user node acl
{
	[[ :$NFSGEN_DEBUG: = *:${NAME}:* || :${NFSGEN_DEBUG}: = *:all:* ]] \
		&& set -x

	typeset user=$1
	typeset node=$2
	typeset acl_t=$3
	typeset ret

	if [[ $user == "" || $node == "" ]]; then
		echo "user, node are not defined."
		case_cleanup $STF_FAIL
	fi

	if [[ $acl_t == *read_xattr* ]]; then
		RUN_CHECK chgusr_exec $user $RUNAT $node $LS \
			> $STF_TMPDIR/$NAME.$$ 2>&1
		ret=$?
	elif [[ $acl_t == *write_xattr* ]]; then
		RUN_CHECK chgusr_exec $user $RUNAT $node $CP $MYTESTFILE attr.1 \
			> $STF_TMPDIR/$NAME.$$ 2>&1
		ret=$?

		if [[ $ret -eq 0 ]]; then
			RUN_CHECK cleanup_test_files basedir \
				|| case_cleanup $STF_FAIL
			RUN_CHECK $TAR xpf@ $ARCHIVEFILE \
				|| case_cleanup $STF_FAIL
		fi
	fi

	return $ret
}

function logname #acl_target user
{
	[[ :$NFSGEN_DEBUG: = *:${NAME}:* || :${NFSGEN_DEBUG}: = *:all:* ]] \
		&& set -x

	typeset acl_target=$1
	typeset user=$2
	typeset ret="RUN_CHECKNEG"

	# To super user, read and write deny permission was override.
	if [[ $user == root || $acl_target == *:allow ]] then
		ret="RUN_CHECK"
	fi

	print $ret
}

function check_chmod_results #node flag acl_target g_usr o_usr
{
	[[ :$NFSGEN_DEBUG: = *:${NAME}:* || :${NFSGEN_DEBUG}: = *:all:* ]] \
		&& set -x

	typeset node=$1
	typeset flg=$2
	typeset acl_target=$2:$3
	typeset g_usr=$4
	typeset o_usr=$5
	typeset log

	if [[ $flg == "owner@" || $flg == "everyone@" ]]; then
		eval "log=$(logname $acl_target $ACL_CUR_USER)" \
			|| case_cleanup $STF_FAIL

		# $log $ACL_CUR_USER:$node $acl_target
		$log operate_node $ACL_CUR_USER $node $acl_target \
			|| case_cleanup $STF_FAIL $STF_TMPDIR/$NAME.$$
	fi
	if [[ $flg == "group@" || $flg == "everyone@" ]]; then
		eval "log=$(logname $acl_target $g_usr)" \
			|| case_cleanup $STF_FAIL

		# $log $g_usr:$node $acl_target
		$log operate_node $g_usr $node $acl_target \
			|| case_cleanup $STF_FAIL $STF_TMPDIR/$NAME.$$
	fi
	if [[ $flg == "everyone@" ]]; then
		eval "log=$(logname $acl_target $o_usr)" \
			|| case_cleanup $STF_FAIL

		# $log $o_usr:$node $acl_target
		$log operate_node $o_usr $node $acl_target \
			|| case_cleanup $STF_FAIL $STF_TMPDIR/$NAME.$$
	fi
}

function test_chmod_basic_access #node g_usr o_usr
{
	[[ :$NFSGEN_DEBUG: = *:${NAME}:* || :${NFSGEN_DEBUG}: = *:all:* ]] \
		&& set -x

	typeset node=${1%/}
	typeset g_usr=$2
	typeset o_usr=$3
	typeset flg acl_p acl_t parent 

	parent=${node%/*}

	for flg in ${a_flag[@]}; do
		for acl_t in "${a_access[@]}"; do
			RUN_CHECK usr_exec $CHMOD A+$flg:$acl_t $node \
				|| case_cleanup $STF_FAIL

			RUN_CHECK $TAR cpf@ $ARCHIVEFILE basedir \
				|| case_cleanup $STF_FAIL

			check_chmod_results "$node" "$flg" \
				"$acl_t" "$g_usr" "$o_usr"

			RUN_CHECK usr_exec $CHMOD A0- $node \
				|| case_cleanup $STF_FAIL
		done
	done
}

function setup_test_files #base_node user group
{
	[[ :$NFSGEN_DEBUG: = *:${NAME}:* || :${NFSGEN_DEBUG}: = *:all:* ]] \
		&& set -x

	typeset base_node=$1
	typeset user=$2
	typeset group=$3

	cleanup_test_files $base_node

	RUN_CHECK $MKDIR -p $base_node || case_cleanup $STF_FAIL
	RUN_CHECK $CHOWN $user:$group $base_node || case_cleanup $STF_FAIL

	RUN_CHECK set_cur_usr $user || case_cleanup $STF_FAIL

	# Prepare all files/sub-dirs for testing.
 
	file0=$base_node/testfile_rm

	dir0=$base_node/testdir_rm

	RUN_CHECK usr_exec $TOUCH $file0 || case_cleanup $STF_FAIL
	RUN_CHECK usr_exec $CHMOD 444 $file0 || case_cleanup $STF_FAIL

	RUN_CHECK usr_exec $RUNAT $file0 $CP $MYTESTFILE attr.0 \
		|| case_cleanup $STF_FAIL

	RUN_CHECK usr_exec $MKDIR -p $dir0 || case_cleanup $STF_FAIL
	RUN_CHECK usr_exec $CHMOD 555 $dir0 || case_cleanup $STF_FAIL

	RUN_CHECK usr_exec $RUNAT $dir0 $CP $MYTESTFILE attr.0 \
		|| case_cleanup $STF_FAIL

	RUN_CHECK usr_exec $CHMOD 777 $base_node || case_cleanup $STF_FAIL
	return 0	
}

function cleanup_test_files #base_node
{
	[[ :$NFSGEN_DEBUG: = *:${NAME}:* || :${NFSGEN_DEBUG}: = *:all:* ]] \
		&& set -x

	typeset base_node=$1

	if [[ -d $base_node ]]; then
		RUN_CHECK $RM -rf $base_node || case_cleanup $STF_FAIL
	elif [[ -e $base_node ]]; then
		RUN_CHECK $RM -f $base_node || case_cleanup $STF_FAIL
	fi

	return 0
}

typeset cwd=$PWD
typeset ARCHIVEFILE=archive.tar

typeset -i i=0
typeset -i j=0
typeset target

cd $TESTDIR
while (( i < ${#users[@]} )); do
	setup_test_files basedir ${users[i]} ${users[((i+1))]}

	j=0
	while (( j < 1 )); do
		eval target=\$file$j	
		test_chmod_basic_access $target \
			"${users[((i+2))]}" "${users[((i+3))]}"

		eval target=\$dir$j	
		test_chmod_basic_access $target \
			"${users[((i+2))]}" "${users[((i+3))]}"

		(( j = j + 1 ))
	done
	
	(( i += 4 ))
done

# Verify that the permission of read_xattr/write_xattr for
# owner/group/everyone are correct.
case_cleanup $STF_PASS
