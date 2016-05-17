Service Manager (Source code)
=============================

The source code to the world's first cross-platform, open source (MIT or LGPL), programming AND scripting language-agnostic solution to system service development.

If you are looking for binaries and instructions on using it, go here:

https://github.com/cubiclesoft/service-manager

Features
--------

* Statically compiled targets.
* Has a liberal open source license.  MIT or LGPL, your choice.
* Sits on GitHub for all of that pull request and issue tracker goodness to easily submit changes and ideas respectively.

Building Service Manager
------------------------

For some strange reason you are overcome by a bizarre desire to build this software product yourself.  There is no ./configure && make here.  Just good old-fashioned shell scripts.

Windows (VC++ command-line):  build.bat

Mac (gcc):  build_mac.sh

Linux and many variants (gcc):  build_nix.sh

You may need to chmod +x or something to get the script to run, but you already knew that.

Testing Service Manager
-----------------------

There is a test PHP-based service that handles 'notify.stop' and 'notify.reload' commands.  It also supports - via the Service Manager PHP SDK - a couple of command-line options to make installation and removal easy:

````
php test_service.php install
php test_service.php uninstall
php test_service.php configdump
````

You can write a test service in whatever language you want though.  It's simple and straightforward.
