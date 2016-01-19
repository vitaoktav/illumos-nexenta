#!/usr/bin/ksh -p
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

###############################################################################
# start __stf_assertion__
#
# ASSERTION: pg_pattern_common_name_002
#
# DESCRIPTION:
#	Verify the property group common name pattern when the explicitly setting
#	all the attributes with punctuation.
#
# STRATEGY:
#	Import the default service and verify the pattern is correct.
#
# end __stf_assertion__
###############################################################################

# Load the helper functions
. ${STF_SUITE}/${STF_EXEC}/../include/templates.kshlib

# Initialize test result
typeset -i RESULT=$STF_PASS

trap cleanup 0 1 2 15

# make sure that the environment is sane - svc.configd is up and running
check_gl_env
if [ $? -ne 0 ] 
then
	echo "--DIAG:
	Invalid test environment - svc.configd not available"

	RESULT=$STF_UNRESOLVED
	exit $RESULT
fi

extract_assertion_info $ME

assertion=pg_pattern_common_name_002

typeset -i test_id=1

readonly registration_template=${MANIFEST}
readonly registration_file=/tmp/pg_pattern_common_name_002.xml
readonly test_service=${SERVICE}

service_exists $test_service
if [ $? -eq 0 ]
then
	service_delete $test_service

	service_exists $test_service
	if [ $? -eq 0 ]
	then
		echo "-- DIAG: [${assertion}]" \
		"	Could not remove service"

		RESULT=$STF_UNRESOLVED
		cleanup
	fi
fi

PG_NAME="foo"
COMMON_NAME="Common name with. -? \! punctuation"

manifest_generate $registration_template \
	CNAME_LANG_1="'C'" \
	COMMON_NAME_1="$COMMON_NAME" \
	CNAME_LANG_2="'ES'" \
	COMMON_NAME_2="spanish common name" > $registration_file

manifest_purgemd5 $registration_file

verify_import pos $registration_file $test_service $OUTFILE $ERRFILE
if [ $? -eq 0 ]; then
	cleanup
fi

echo "--INFO: Validate the property group pattern common name"
pgn=${PG_PREFIX_NT}${PG_NAME}
verify_prop $test_service $pgn/common_name_C ustring "Common name with. -? ! punctuation"

service_cleanup $test_service

trap 0 1 2 15

exit $RESULT
