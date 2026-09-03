#!/usr/bin/env bash
set -e

TOOLCHAIN_DIR="$(cd "$(dirname "$0")/.." && pwd)/toolchain/fenwick-toolchain"
BIN_DIR="$TOOLCHAIN_DIR/bin"
TEMP_DIR="$TOOLCHAIN_DIR/temp"
INSTALL_DIR="$TOOLCHAIN_DIR/opt"

mkdir -p "$BIN_DIR" "$TEMP_DIR" "$INSTALL_DIR"

export PATH="$BIN_DIR:$PATH"

BINUTILS_VER="2.44"
GCC_VER="14.2.0"
TARGET="x86_64-elf"

run_step() {
    local label="$1"
    shift
    local done_file=$(mktemp)
    local status_file=$(mktemp)
    
    (
        set +e
        "$@"
        local status=$?
        echo $status > "$status_file"
        touch "$done_file"
        exit $status
    ) > /dev/null 2>&1 &
    local pid=$!
    
    local i=0
    local width=50
    echo -n "$label ["
    while kill -0 "$pid" 2>/dev/null && [ $i -lt $width ]; do
        echo -n "#"
        sleep 0.2
        i=$((i+1))
    done
    while [ $i -lt $width ]; do
        echo -n "#"
        i=$((i+1))
    done
    echo "]"
    
    wait "$pid"
    rm -f "$done_file" "$status_file"
}

cd "$TEMP_DIR"

if [ ! -f "binutils-$BINUTILS_VER.tar.gz" ]; then
    curl -sLO "https://ftp.gnu.org/gnu/binutils/binutils-$BINUTILS_VER.tar.gz"
fi

if [ ! -f "gcc-$GCC_VER.tar.gz" ]; then
    curl -sLO "https://ftp.gnu.org/gnu/gcc/gcc-$GCC_VER/gcc-$GCC_VER.tar.gz"
fi

if [ ! -d "binutils-$BINUTILS_VER" ]; then
    tar -xzf binutils-$BINUTILS_VER.tar.gz
fi

if [ ! -d "gcc-$GCC_VER" ]; then
    tar -xzf gcc-$GCC_VER.tar.gz
fi

if [ ! -f "$BIN_DIR/$TARGET-ld" ]; then
    mkdir -p binutils-build
    cd binutils-build
    ../binutils-$BINUTILS_VER/configure --target=$TARGET --prefix="$INSTALL_DIR" --with-sysroot --disable-nls --disable-werror > /dev/null 2>&1
    run_step "fenwick-binutils" make -j$(nproc 2>/dev/null || echo 4)
    run_step "fenwick-binutils-install" make install
    cd ..
fi

if [ ! -f "$BIN_DIR/$TARGET-gcc" ]; then
    mkdir -p gcc-build
    cd gcc-build
    ../gcc-$GCC_VER/configure --target=$TARGET --prefix="$INSTALL_DIR" --disable-nls --enable-languages=c --without-headers --with-newlib --disable-shared --disable-libssp --disable-libstdcxx-pch --disable-multilib > /dev/null 2>&1
    run_step "fenwick-gcc" make all-gcc -j$(nproc 2>/dev/null || echo 4)
    run_step "fenwick-gcc-install" make install-gcc
    cd ..
fi

cat > "$BIN_DIR/fenwick-gcc" << 'EOF'
#!/usr/bin/env bash
INSTALL_DIR="$(cd "$(dirname "$0")" && pwd)"
exec "$INSTALL_DIR/x86_64-elf-gcc" -ffreestanding -nostdlib -nostartfiles "$@"
EOF

cat > "$BIN_DIR/fenwick-ldd" << 'EOF'
#!/usr/bin/env bash
INSTALL_DIR="$(cd "$(dirname "$0")" && pwd)"
exec "$INSTALL_DIR/x86_64-elf-ld" "$@"
EOF

cat > "$BIN_DIR/fenwind-objcopy" << 'EOF'
#!/usr/bin/env bash
INSTALL_DIR="$(cd "$(dirname "$0")" && pwd)"
exec "$INSTALL_DIR/x86_64-elf-objcopy" "$@"
EOF

cat > "$BIN_DIR/fenwick-nasm" << 'EOF'
#!/usr/bin/env bash
INSTALL_DIR="$(cd "$(dirname "$0")" && pwd)"
exec nasm -f elf64 "$@"
EOF

chmod +x "$BIN_DIR/fenwick-gcc" "$BIN_DIR/fenwick-ldd" "$BIN_DIR/fenwind-objcopy" "$BIN_DIR/fenwick-nasm"

TOOLCHAIN_BIN_WIN="$(cygpath -w "$BIN_DIR")"
echo "$TOOLCHAIN_BIN_WIN"
