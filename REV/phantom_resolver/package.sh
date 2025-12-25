#!/bin/bash
# Create distribution package for contestants

set -e

DIST_DIR="dist"
PACKAGE_NAME="phantom_resolver_challenge"

echo "[*] Creating challenge package..."

# Create temporary packaging directory
rm -rf /tmp/$PACKAGE_NAME
mkdir -p /tmp/$PACKAGE_NAME

# Copy distribution files
cp $DIST_DIR/libmonitor.so /tmp/$PACKAGE_NAME/
cp $DIST_DIR/server_daemon.template /tmp/$PACKAGE_NAME/
cp $DIST_DIR/README.md /tmp/$PACKAGE_NAME/

# Create the ZIP file
cd /tmp
zip -r $PACKAGE_NAME.zip $PACKAGE_NAME/

# Move back to challenge directory
mv $PACKAGE_NAME.zip -

echo "[+] Package created: $PACKAGE_NAME.zip"
echo ""
echo "Contents:"
unzip -l $PACKAGE_NAME.zip
echo ""
echo "Upload this ZIP to CTFd!"

# Cleanup
rm -rf /tmp/$PACKAGE_NAME
