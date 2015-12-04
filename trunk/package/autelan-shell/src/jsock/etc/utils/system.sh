#!/bin/bash

main() {
	local err=0;

	eval "($@) &> /dev/null"; err=$?

	echo ${err}

	return ${err}
}

main "$@"
