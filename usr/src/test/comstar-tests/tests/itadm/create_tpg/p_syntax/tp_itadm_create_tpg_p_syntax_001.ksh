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

#
# A test purpose file to test functionality of the create-tpg
# subfunction of the itadm command
#

# __stc_assertion_start
#
# ID: itadm_create_tpg_p_syntax_001
#
# DESCRIPTION:
#	itadm create-tpg command successfully creates the specified
#	target portal group with all valid combinations of options
#
# STRATEGY:
# 	Create a dynamic test case that generates command and options
# 	combinations with a scope of the valid argument.
#
#	Setup:
#	 	Plumb an network interface with a IP address if this matter
#	Test:
#		1. itadm create-tpg <tpg-tag> <IP-address:port>
#
#	           Valid arguments:
#
#		   <tpg-tag>
#			1
#			30000
#			A String of 255 characters
#		   <IP-address:port>
#			Plumbed IP address (Use default port 3620)
#			Plumbed IP address:3621
#
#		2. itadm list-tpg to make sure the correct information
#		   is reported
#	Cleanup:
#		Delete the created target portal group
#
#	STRATEGY_NOTES:
#
#
# TESTABILITY: explicit
#
# AUTHOR: bobbie.long@sun.com zheng.he@sun.com
#
# REVIEWERS:
#
# ASSERTION_SOURCE:
#	http://www.opensolaris.org/os/project/iser/itadm_1m_v4.pdf
#
# TEST_AUTOMATION_LEVEL: automated
#
# STATUS: IN_PROGRESS
#
# COMMENTS:
#
# __stc_assertion_end
#

CMD="itadm_create POS tpg ^SET_1 ^SET_2"

SET_1=" \
        1		| \ 
        30000		| \ 
        ^STR_255	| \
"

SET_2=" \
        127.0.0.1	| \
        127.0.0.1:3261	| \
"

STR_255=" \
Total_255_characters_aaaaaaaaaaaaaaaaaaaaaaaaaaaaa\
aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\
aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\
aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\
aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\
aaaaa"

set -A CMD_ARR

function itadm_create_tpg_p_syntax
{
        cti_pass
        typeset -i i
        (( i = ${TET_TPNUMBER} - 1 ))

        tc_id="itadm_create_tpg_p_syntax_${TET_TPNUMBER}"
	tc_desc="itadm create-tpg command successfully creates the specified"
	tc_desc="${tc_desc} target portal group with { "
	tc_desc="${tc_desc} ${CMD_ARR[${i}]} }"

	print_test_case $tc_id - $tc_desc

        ${CMD_ARR[${i}]}

        tp_cleanup

}

auto_generate_tp "CMD" "CMD_ARR" "itadm_create_tpg_p_syntax"


