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
# ident	"@(#)tp_set_004.ksh	1.3	08/06/11 SMI"
#

#
# 'sharemgr set' test case
#

#__stc_assertion_start
#
#ID: set_004
#
#DESCRIPTION:
#
#	Set and change the nfs index property.
#
#STRATEGY:
#
#	Setup:
#		- Create share group with nfs protocol.
#	Test:
#		- Set the index property to one file name.
#		- Change the index property to another file name.
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
function set_004 {
	tet_result PASS
	tc_id="set_004"
	tc_desc="Set and modify the nfs index property"
	cmd_list=""
	unset GROUPS
	print_test_case $tc_id - $tc_desc
	#
	# Setup
	#
	# Create share group with nfs protocol
	create test_group_1 -P nfs

	#
	# Set and then modify the index property. (Dry run first then the real
	# thing in each case.)
	#
	set_ POS test_group_1 -P nfs -n -p index=\"test_file_aaa\"
	set_ POS test_group_1 -P nfs -p index=\"test_file_aaa\"
	set_ POS test_group_1 -P nfs -n -p index=\"test_file_bbb\"
	set_ POS test_group_1 -P nfs -p index=\"test_file_bbb\"

	#
	# Cleanup
	#
	# Delete all test groups
	delete_all_test_groups
	report_cmds $tc_id POS
}
