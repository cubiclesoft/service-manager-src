Service Manager (Source code)
=============================

The source code to the world's first cross-platform, open source (MIT or LGPL), programming AND scripting language-agnostic solution to system service development.

If you are looking for binaries and instructions on using it, go here:

https://github.com/cubiclesoft/service-manager

Features
--------

* Statically compiled targets.
* Local build and global install option for some platforms.
* Has a liberal open source license.  MIT or LGPL, your choice.
* Sits on GitHub for all of that pull request and issue tracker goodness to easily submit changes and ideas respectively.

Building Service Manager
------------------------

For some strange reason you are overcome by a bizarre desire to build this software product yourself.  There is no ./configure && make here.  Just good old-fashioned shell scripts.

Windows (VC++ command-line):  build.bat

Mac (gcc):  build_mac.sh

Linux and many variants (gcc):  build_nix.sh

You may need to chmod +x or something to get the script to run, but you already knew that.

Building and Installing Locally
-------------------------------

The primary target platform for Service Manager is the unified x86/x64 architecture (Windows, Mac, and Linux).  However, for *NIX platforms on other architectures (e.g. the fairly popular ARM architecture), Service Manager can be compiled and optionally installed on the target system.

Simply run:

```
./build_nix.sh
sudo ./install_nix.sh
```

Which will put the compiled binary at `/usr/local/bin/servicemanager` and the base service platform files into `/usr/share/servicemanager/`.

When Service Manager is installed as described above, the Service Manager SDK will generally prefer using the installed version instead of binaries that are bundled with a project.

Testing Service Manager
-----------------------

There is a test PHP-based service that handles 'notify.stop' and 'notify.reload' commands.  It also supports - via the Service Manager PHP SDK - a couple of command-line options to make installation and removal easy:

````
php test_service.php install
php test_service.php start
php test_service.php stop
php test_service.php uninstall
php test_service.php configdump
````

You can, of course, write a test service in whatever language you want though.  It's simple and straightforward.
