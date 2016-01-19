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
# ASSERTION: svccfg_import_004
#
# DESCRIPTION:
#	Calling the "import file" subcommand with an invalid file
#	result in a diagnostic message being sent to stderr and the
#	command exiting with an exit status of 1.  Invalid file
#	should include the following:  empty file, regular text file,
#	binary file, non-existent file, directory, device.
#
# STRATEGY:
#	Test the import subcommand using each of the types of files
#	listed in the assertion above.  We don't check the error
#	output generated by import because it is likely to change
#	and depends on the file type.  Checking would likely require
#	more maintenance of this test than is warranted.
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
	
	rm -f $OUTFILE $ERRFILE ${TEST_FILE}

	exit $RESULT
}

trap cleanup 0 1 2 15

# make sure that the environment is sane - svc.configd  is up and running
check_gl_env
[[ $? -ne 0 ]] && {
	echo "--DIAG: 
	     	Invalid test environment - svc.configd  not available"

        RESULT=$STF_UNRESOLVED
	exit $RESULT
}

assertion=svccfg_import_004

# extract and print assertion information from this source script.
extract_assertion_info $ME

# Test #1: import an empty file

typeset -i TEST_RESULT=$STF_PASS

echo "--INFO: Import an empty file"

typeset TEST_FILE=/tmp/empty.$$

rm -f ${TEST_FILE}

touch ${TEST_FILE} > $OUTFILE 2>$ERRFILE
ret=$?
[[ $ret != 0 ]] && {
	echo "--DIAG: [${assertion}, test 1]
	could not create empty file ${TEST_FILE}
	error output is $(cat $ERRFILE)"

	RESULT=$(update_result $STF_UNRESOLVED $RESULT)
	exit $RESULT
}

svccfg import ${TEST_FILE} > $OUTFILE 2>$ERRFILE
ret=$?
[[ $ret -ne 1 ]] &&  {
	echo "--DIAG: [${assertion}, test 1]
	svccfg import expected to return 1, got $ret"

	TEST_RESULT=$STF_FAIL
}

# Verify that nothing in stdout - this is a non-fatal error
[[ -s $OUTFILE ]] &&  {
	echo "--DIAG: [${assertion}, test 1]
	stdout not expected, but got $(cat $OUTFILE)"

	TEST_RESULT=$STF_FAIL
}

echo "--CHECK: error output is $(cat $ERRFILE)"

rm -f $ERRFILE $OUTFILE ${TEST_FILE}

print_result $TEST_RESULT
RESULT=$(update_result $TEST_RESULT $RESULT)


# Test #2: import a regular text file

typeset -i TEST_RESULT=$STF_PASS

echo "--INFO: Import a regular text file"

TEST_FILE=$ME
[[ ! -f "$ME" ]] && {
	echo "--DIAG: [${assertion}, test 3]
	$ME expected to exist but does not"

	RESULT=$(update_result $STF_UNRESOLVED $RESULT)

	exit $RESULT
}
	

# Use this file as a regular text file

svccfg import ${TEST_FILE} > $OUTFILE 2>$ERRFILE
ret=$?
[[ $ret -ne 1 ]] &&  {
	echo "--DIAG: [${assertion}, test 2]
	svccfg import expected to return 1, got $ret"

	TEST_RESULT=$STF_FAIL
}

# Verify that nothing in stdout - this is a non-fatal error
[[ -s $OUTFILE ]] &&  {
	echo "--DIAG: [${assertion}, test 2]
	stdout not expected, but got $(cat $OUTFILE)"

	TEST_RESULT=$STF_FAIL
}

echo "--CHECK: error output is $(cat $ERRFILE)"

rm -f $ERRFILE $OUTFILE 

print_result $TEST_RESULT
RESULT=$(update_result $TEST_RESULT $RESULT)


# Test #3: import a binary file

typeset -i TEST_RESULT=$STF_PASS

echo "--INFO: Import a binary file"

# Use svc.configd as a binary file

TEST_FILE=/lib/svc/bin/svc.configd
[[ ! -f "${TEST_FILE}" ]] && {
	echo "--DIAG: [${assertion}, test 3]
	${TEST_FILE} expected to exist but does not"

	RESULT=$(update_result $STF_UNRESOLVED $RESULT)

	exit $RESULT
}
	

svccfg import ${TEST_FILE} > $OUTFILE 2>$ERRFILE
ret=$?
[[ $ret -ne 1 ]] &&  {
	echo "--DIAG: [${assertion}, test 3]
	svccfg import expected to return 1, got $ret"

	TEST_RESULT=$STF_FAIL
}

# Verify that nothing in stdout - this is a non-fatal error
[[ -s $OUTFILE ]] &&  {
	echo "--DIAG: [${assertion}, test 3]
	stdout not expected, but got $(cat $OUTFILE)"

	TEST_RESULT=$STF_FAIL
}

echo "--CHECK: error output is $(cat $ERRFILE)"

rm -f $ERRFILE $OUTFILE 

print_result $TEST_RESULT
RESULT=$(update_result $TEST_RESULT $RESULT)

# Test #4: import a non-existent file

typeset -i TEST_RESULT=$STF_PASS

echo "--INFO: Import a non-existent file"

typeset TEST_FILE=/tmp/empty.$$
rm -f ${TEST_FILE}

[[ -a "${TEST_FILE} " ]] && {
	echo "--DIAG: [${assertion}, test 4]
	$EMPTY_FILE not expected to exist but does"

	RESULT=$(update_result $STF_UNRESOLVED $RESULT)

	exit $RESULT
}
	


svccfg import ${TEST_FILE} > $OUTFILE 2>$ERRFILE
ret=$?
[[ $ret -ne 1 ]] &&  {
	echo "--DIAG: [${assertion}, test 4]
	svccfg import expected to return 1, got $ret"

	TEST_RESULT=$STF_FAIL
}

# Verify that nothing in stdout - this is a non-fatal error
[[ -s $OUTFILE ]] &&  {
	echo "--DIAG: [${assertion}, test 4]
	stdout not expected, but got $(cat $OUTFILE)"

	TEST_RESULT=$STF_FAIL
}

echo "--CHECK: error output is $(cat $ERRFILE)"

rm -f $ERRFILE $OUTFILE 

print_result $TEST_RESULT
RESULT=$(update_result $TEST_RESULT $RESULT)

# Test #5: import a directory

typeset -i TEST_RESULT=$STF_PASS

echo "--INFO: Import a directory"

# Use $STF_SUITE directory

[[ ! -d "$STF_SUITE" ]] && {
	echo "--DIAG: [${assertion}, test 5]
	$STF_SUITE did not test as a directory"

	RESULT=$(update_result $STF_UNRESOLVED $RESULT)

	exit $RESULT
}
	

svccfg import $STF_SUITE > $OUTFILE 2>$ERRFILE
ret=$?
[[ $ret -ne 1 ]] &&  {
	echo "--DIAG: [${assertion}, test 5]
	svccfg import expected to return 1, got $ret"

	TEST_RESULT=$STF_FAIL
}

# Verify that nothing in stdout - this is a non-fatal error
[[ -s $OUTFILE ]] &&  {
	echo "--DIAG: [${assertion}, test 5]
	stdout not expected, but got $(cat $OUTFILE)"

	TEST_RESULT=$STF_FAIL
}

echo "--CHECK: error output is $(cat $ERRFILE)"

rm -f $ERRFILE $OUTFILE 

print_result $TEST_RESULT
RESULT=$(update_result $TEST_RESULT $RESULT)

# Test #6: import a device file

typeset -i TEST_RESULT=$STF_PASS

echo "--INFO: Import a device file"
device="/dev/random"

svccfg import $device > $OUTFILE 2>$ERRFILE
ret=$?
[[ $ret -ne 1 ]] &&  {
	echo "--DIAG: [${assertion}, test 6]
	svccfg import expected to return 1, got $ret"

	TEST_RESULT=$STF_FAIL
}

# Verify that nothing in stdout - this is a non-fatal error
[[ -s $OUTFILE ]] &&  {
	echo "--DIAG: [${assertion}, test 6]
	stdout not expected, but got $(cat $OUTFILE)"

	TEST_RESULT=$STF_FAIL
}

echo "--CHECK: error output is $(cat $ERRFILE)"

rm -f $ERRFILE $OUTFILE 

print_result $TEST_RESULT
RESULT=$(update_result $TEST_RESULT $RESULT)

exit $RESULT
