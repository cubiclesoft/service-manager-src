#!/bin/bash

# Check for root privileges.
if ! [ $(id -u) = 0 ]; then
	echo "The installer requires root privileges."

	exit 1
fi

# Install Service Manager to FHS locations.
cp servicemanager_nix /usr/local/bin/servicemanager
chown root /usr/local/bin/servicemanager
chgrp root /usr/local/bin/servicemanager
chmod 755 /usr/local/bin/servicemanager

mkdir -p /usr/share/servicemanager
cp servicemanager_nix.* /usr/share/servicemanager
chown -R root /usr/share/servicemanager
chgrp -R root /usr/share/servicemanager
chmod 755 /usr/share/servicemanager
chmod 644 /usr/share/servicemanager/servicemanager_nix.*
