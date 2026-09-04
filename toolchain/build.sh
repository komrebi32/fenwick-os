#!/usr/bin/env bash
set -e

echo "Checking system dependencies..."

NEEDED_PKGS=""
command -v nasm >/dev/null 2>&1 || NEEDED_PKGS="$NEEDED_PKGS nasm"
command -v grub-mkrescue >/dev/null 2>&1 || NEEDED_PKGS="$NEEDED_PKGS grub2-common grub-pc-bin xorriso"
command -v curl >/dev/null 2>&1 || NEEDED_PKGS="$NEEDED_PKGS curl"
command -v make >/dev/null 2>&1 || NEEDED_PKGS="$NEEDED_PKGS make"

if [ -n "$NEEDED_PKGS" ]; then
    echo "Installing: $NEEDED_PKGS"
    sudo apt-get update -qq
    sudo apt-get install -y -qq $NEEDED_PKGS
fi

TOOLCHAIN_DIR="$(cd "$(dirname "$0")/.." && pwd)/toolchain/fenwick-toolchain"
BIN_DIR="$TOOLCHAIN_DIR/bin"
TEMP_DIR="$TOOLCHAIN_DIR/temp"
INSTALL_DIR="$TOOLCHAIN_DIR/opt"

mkdir -p "$BIN_DIR" "$TEMP_DIR" "$INSTALL_DIR"

export PATH="$BIN_DIR:$PATH"

BINUTILS_VER="2.44"
GCC_VER="14.2.0"
TARGET="x86_64-elf"
RUST_TARGET="x86_64-unknown-none"

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

if ! command -v cargo >/dev/null 2>&1; then
    echo "Installing Rust toolchain..."
    run_step "fenwick-rust" bash -c 'curl --proto "=https" --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y --default-toolchain stable --profile minimal'
    source "$HOME/.cargo/env" || true
    rustup target add "$RUST_TARGET"
fi

cd "$TEMP_DIR"

if [ ! -f "binutils-$BINUTILS_VER.tar.gz" ]; then
    echo "Downloading binutils..."
    curl -sLO "https://ftp.gnu.org/gnu/binutils/binutils-$BINUTILS_VER.tar.gz"
fi

if [ ! -f "gcc-$GCC_VER.tar.gz" ]; then
    echo "Downloading gcc..."
    curl -sLO "https://ftp.gnu.org/gnu/gcc/gcc-$GCC_VER/gcc-$GCC_VER.tar.gz"
fi

if [ ! -d "binutils-$BINUTILS_VER" ]; then
    echo "Extracting binutils..."
    tar -xzf binutils-$BINUTILS_VER.tar.gz
fi

if [ ! -d "gcc-$GCC_VER" ]; then
    echo "Extracting gcc..."
    tar -xzf gcc-$GCC_VER.tar.gz
fi

if [ ! -f "$BIN_DIR/$TARGET-ld" ]; then
    echo "Building binutils..."
    mkdir -p binutils-build
    cd binutils-build
    ../binutils-$BINUTILS_VER/configure --target=$TARGET --prefix="$INSTALL_DIR" --with-sysroot --disable-nls --disable-werror > /dev/null 2>&1
    run_step "fenwick-binutils" make -j$(nproc 2>/dev/null || echo 4)
    run_step "fenwick-binutils-install" make install
    cd ..
fi

if [ ! -f "$BIN_DIR/$TARGET-gcc" ]; then
    echo "Building GCC..."
    mkdir -p gcc-build
    cd gcc-build
    ../gcc-$GCC_VER/configure --target=$TARGET --prefix="$INSTALL_DIR" --disable-nls --enable-languages=c --without-headers --with-newlib --disable-shared --disable-libssp --disable-libstdcxx-pch --disable-multilib > /dev/null 2>&1
    run_step "fenwick-gcc" make all-gcc -j$(nproc 2>/dev/null || echo 4)
    run_step "fenwick-gcc-install" make install-gcc
    cd ..
fi

echo "Creating wrapper scripts..."

cat > "$BIN_DIR/fenwick-gcc" << 'EOF'
#!/usr/bin/env bash
BIN_DIR="$(cd "$(dirname "$0")" && pwd)"
exec "$BIN_DIR/../opt/bin/x86_64-elf-gcc" -ffreestanding -nostdlib -nostartfiles "$@"
EOF

cat > "$BIN_DIR/fenwick-ldd" << 'EOF'
#!/usr/bin/env bash
BIN_DIR="$(cd "$(dirname "$0")" && pwd)"
exec "$BIN_DIR/../opt/bin/x86_64-elf-ld" "$@"
EOF

cat > "$BIN_DIR/fenwind-objcopy" << 'EOF'
#!/usr/bin/env bash
BIN_DIR="$(cd "$(dirname "$0")" && pwd)"
exec "$BIN_DIR/../opt/bin/x86_64-elf-objcopy" "$@"
EOF

cat > "$BIN_DIR/fenwick-nasm" << 'EOF'
#!/usr/bin/env bash
BIN_DIR="$(cd "$(dirname "$0")" && pwd)"
exec nasm -f elf64 "$@"
EOF

cat > "$BIN_DIR/fenwick-cargo" << 'EOF'
#!/usr/bin/env bash
if [ -x "$HOME/.cargo/bin/cargo" ]; then
    exec "$HOME/.cargo/bin/cargo" "$@"
elif command -v cargo >/dev/null 2>&1; then
    exec cargo "$@"
else
    echo "cargo not found" >&2
    exit 1
fi
EOF

chmod +x "$BIN_DIR/fenwick-gcc" "$BIN_DIR/fenwick-ldd" "$BIN_DIR/fenwind-objcopy" "$BIN_DIR/fenwick-nasm" "$BIN_DIR/fenwick-cargo"

echo ""
echo "Toolchain ready!"
echo "BIN_DIR=$BIN_DIR"
echo ""
echo "Run: export PATH=\"$BIN_DIR:\$PATH\""
