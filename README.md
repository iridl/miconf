# README

## Dependencies

* miconf-multi requires python and ply

## Build and Install

* Make sure the following libraries are installed, for example:

    * Using yum:

            sudo yum install readline-devel ncurses-devel

    * Using apt-get:

            sudo apt-get install libreadline-dev
            sudo apt-get install libncurses5-dev
	

* Clone miconf repository with flag `--recursive`:

        git clone --recursive git@bitbucket.org:iridl/miconf.git

* Build and install as follows:

        make
        sudo make PREFIX=/usr/local install