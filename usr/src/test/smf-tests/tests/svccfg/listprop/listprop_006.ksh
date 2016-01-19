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
# Copyright 2008 Sun Microsystems, Inc.  All rights reserved.
# Use is subject to license terms.
#

###############################################################################
# start __stf_assertion__
#
# ASSERTION: svccfg_listprop_006
#
# DESCRIPTION:
#	The 'listprop' subcommand expects at most one argument, 
#	a glob pattern.  If more than one argument is provided 
#	to 'listprop' than a diagnostic message will be sent to 
#	stderr and the exit status of 1 is returned.
#
# end __stf_assertion__
###############################################################################


# First STF library
. ${STF_TOOLS}/include/stf.kshlib

# Load GL library
. ${STF_SUITE}/include/gltest.kshlib


readonly ME=$(whence -p ${0})
readonly MYLOC=$(dirname ${ME})

# Initialize test result 
typeset -i RESULT=$STF_PASS


function cleanup {

        # Note that $TEST_SERVICE may or may not exist so don't check
	# results.  Just make sure the service is gone.
	service_delete $TEST_SERVICE

	service_exists ${TEST_SERVICE}
	[[ $? -eq 0 ]] && {
		echo "--DIAG: [${assertion}, cleanup]
		service ${TEST_SERVICE} should not exist in
		repository after being deleted, but does"

		RESULT=$(update_result $STF_UNRESOLVED $RESULT)
	}
	
	rm -f $OUTFILE $ERRFILE $CMDFILE

	exit $RESULT
}

trap cleanup 0 1 2 15

# make sure that the environment is sane - svc.configd is up and running
check_gl_env
[[ $? -ne 0 ]] && {
	echo "--DIAG: 
	Invalid test environment - svc.configd not available"

	RESULT=$STF_UNRESOLVED 
	exit $RESULT
}

# extract and print assertion information from this source script.
extract_assertion_info $ME

assertion=svccfg_listprop_006

cat << EOF >$CMDFILE
add $TEST_SERVICE
select $TEST_SERVICE
addpg ${TEST_PROPERTY} framework
EOF

svccfg -f $CMDFILE > $OUTFILE 2>$ERRFILE
ret=$?
[[ $ret -ne 0 ]] && {
	echo "--DIAG: [${assertion}]
	svccfg expected to return 0, got $ret
	error output is $(cat $ERRFILE)"

	RESULT=$(update_result $STF_UNRESOLVED $RESULT)
	exit $RESULT
}

cat << EOF >$CMDFILE
select $TEST_SERVICE
listprop * extraoption_1 extraoption_2 extraoption_3
EOF

svccfg -f $CMDFILE > $OUTFILE 2>$ERRFILE
ret=$?

# Verify that the return value is as expected - non-fatal error
[[ $ret -ne 1 ]] &&  {
	echo "--DIAG: [${assertion}]
	svccfg add expected to return 1, got $ret"

 	RESULT=$STF_FAIL
}

# Verify that nothing in stdout - non-fatal error
[[ -s $OUTFILE ]] &&  {
	echo "--DIAG: [${assertion}]
        stdout not expected, but got $(cat $OUTFILE)"

 	RESULT=$STF_FAIL
}


# Verify that message in stderr - non-fatal error
if ! egrep -s "$SYNTAX_ERRMSG" $ERRFILE
then
	echo "--DIAG: [${assertion}]
	Expected error message \"$SYNTAX_ERRMSG\"
	but got \"$(cat $ERRFILE)\""

	RESULT=$STF_FAIL
fi

rm -f $ERRFILE $OUTFILE $CMDFILE

print_result $RESULT

exit $RESULT
