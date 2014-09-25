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
# Copyright 2008 Sun Microsystems, Inc.  All rights reserved.
# Use is subject to license terms.
#
# ident	"@(#)tp_set_share_006.ksh	1.4	08/06/11 SMI"
#

#
# sharemgr set_share test case
#

#__stc_assertion_start
#
#ID: set_share006
#
#DESCRIPTION:
#
#	Verify clearing resource name doesn't affect description.
#
#STRATEGY:
#
#	Setup:
#		- Create share group with default options.
#		- Add share to share group with resource name and description
#		  specified.
#		- Verify (by new and legacy* methods) that the share is indeed
#		  shared and has the default options.
#	Test:
#		- Clear resource name and for the share.
#		- Verify (by new and legacy* methods) that the share is still
#		  shared, has the default options, and has the expected
#		  resource name and description.
#	Cleanup:
#		- Forcibly delete all share groups.
#
#	STRATEGY_NOTES:
#		- * Legacy methods will be used so long as they are still
#		  present.
#		- Return status is checked for all share-related commands
#		  executed.
#		- For all commands that modify the share configuration, the
#		  associated reporting commands will be executed and output
#		  checked to verify the expected changes have occurred.
#
#KEYWORDS:
#
#	move-share
#
#TESTABILITY: explicit
#
#AUTHOR: andre.molyneux@sun.com
#
#REVIEWERS: TBD
#
#TEST_AUTOMATION_LEVEL: automated
#
#CODING_STATUS: COMPLETE
#
#__stc_assertion_end
function set_share006 {
	tet_result PASS
	tc_id="set_share006"
	tc_desc="Verify clearing resource name doesn't affect description"
	cmd_list=""
	unset GROUPS
	print_test_case $tc_id - $tc_desc
	#
	# Setup
	#
	# Create and populate initial share group
	create test_group_1 -P nfs
	add_share POS test_group_1 "-d \"init desc\" -r init_rsrc" ${MP[0]}
	#
	# Perform set-share operation.  (Dry run then the real thing.)
	#
	set_share POS ${MP[0]} "-n -r \"\""
	set_share POS ${MP[0]} "-r \"\""
	#
	# Cleanup
	#
	# Delete all test groups
	delete_all_test_groups
	report_cmds $tc_id POS
}
