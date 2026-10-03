#!/bin/bash
set -e

ROOTFS_DIR="${1:-./rootfs}"

echo "=== SandBoxX Minimal Rootfs Setup ==="
echo "Target directory: $ROOTFS_DIR"

mkdir -p "$ROOTFS_DIR"/{bin,lib,lib64,usr/bin,usr/lib,tmp,proc,dev,etc}
chmod 1777 "$ROOTFS_DIR/tmp"

# Populate minimal /etc files
echo "sandboxx-jail" > "$ROOTFS_DIR/etc/hostname"
echo "127.0.0.1 localhost sandboxx-jail" > "$ROOTFS_DIR/etc/hosts"
echo "root:x:0:0:root:/root:/bin/sh" > "$ROOTFS_DIR/etc/passwd"
echo "nogroup:x:65534:" > "$ROOTFS_DIR/etc/group"

# Copy busybox or bash if present
if command -v busybox &>/dev/null; then
    cp "$(command -v busybox)" "$ROOTFS_DIR/bin/busybox"
    ln -sf /bin/busybox "$ROOTFS_DIR/bin/sh"
    ln -sf /bin/busybox "$ROOTFS_DIR/bin/ls"
    ln -sf /bin/busybox "$ROOTFS_DIR/bin/echo"
    ln -sf /bin/busybox "$ROOTFS_DIR/bin/cat"
    echo "Installed busybox into $ROOTFS_DIR/bin"
fi

echo "Rootfs initialized at $ROOTFS_DIR"
