# Native LCD4Linux Driver Compilation Guide

This guide explains how to compile lcd4linux from source with the custom ESP32-C3 OLED driver.

## Prerequisites

### Install Build Dependencies

#### Debian/Ubuntu:
```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    autoconf \
    automake \
    libtool \
    pkg-config \
    libusb-dev \
    libgd-dev \
    libncurses5-dev \
    python3-dev \
    git
```

#### Fedora/RHEL:
```bash
sudo dnf install -y \
    gcc \
    make \
    autoconf \
    automake \
    libtool \
    pkgconfig \
    libusb-devel \
    gd-devel \
    ncurses-devel \
    python3-devel \
    git
```

#### Arch Linux:
```bash
sudo pacman -S \
    base-devel \
    autoconf \
    automake \
    libtool \
    pkgconfig \
    libusb \
    gd \
    ncurses \
    python \
    git
```

## Step-by-Step Compilation

### 1. Download LCD4Linux Source

```bash
# Clone the official repository
cd ~/Projects
git clone https://github.com/TangoCash/lcd4linux.git
cd lcd4linux

# Or download the release tarball
wget https://github.com/TangoCash/lcd4linux/archive/refs/heads/master.tar.gz
tar xzf master.tar.gz
cd lcd4linux-master
```

### 2. Add ESP32-OLED Driver

```bash
# Copy the driver file
cp /home/wanleung/Projects/linux4lcd-esp-c3/driver/drv_ESP32OLED.c drv_ESP32OLED.c
```

### 3. Modify Build Configuration

Edit `configure.ac` or `configure.in`:

```bash
# Open the file
nano configure.ac
```

Find the section with driver definitions (around line 600-800) and add:

```
# ESP32-OLED driver
AC_ARG_WITH(
  esp32oled,
  [  --with-esp32oled        ESP32-C3 with I2C OLED display [[default=yes]]],
  with_esp32oled="$withval",
  with_esp32oled="yes"
)
if test "$with_esp32oled" = "yes"; then
  TEXT="yes"
  GRAPHIC="yes"
  SERIAL="yes"
  DRIVERS="$DRIVERS drv_ESP32OLED.o"
  AC_DEFINE(WITH_ESP32OLED,1,[ESP32-C3 OLED driver])
fi
```

### 4. Modify drivers.m4

Edit `drivers.m4` and add ESP32OLED to the driver list:

```bash
nano drivers.m4
```

Add to the driver list:
```
  ESP32OLED,
```

### 5. Register the Driver

Edit `drv.c`:

```bash
nano drv.c
```

Add to the includes section:
```c
#ifdef WITH_ESP32OLED
extern DRIVER drv_ESP32OLED;
#endif
```

Add to the driver list array:
```c
#ifdef WITH_ESP32OLED
    &drv_ESP32OLED,
#endif
```

### 6. Configure and Build

```bash
# Generate configure script
./bootstrap

# Or if bootstrap doesn't exist:
aclocal
autoheader
automake --add-missing
autoconf

# Configure with ESP32OLED driver
./configure \
    --with-esp32oled \
    --with-plugins="all" \
    --prefix=/usr/local

# Build
make -j$(nproc)

# Check if driver is included
./lcd4linux -l | grep ESP32

# Install
sudo make install
```

### 7. Verify Installation

```bash
# Check version
lcd4linux -V

# List available drivers (should show ESP32OLED)
lcd4linux -l

# Test configuration
lcd4linux -F -f /path/to/lcd4linux-native.conf
```

## Alternative: Quick Patch Method

### Create a Patch File

```bash
cd ~/Projects/linux4lcd-esp-c3
./create-lcd4linux-patch.sh
```

This creates `lcd4linux-esp32oled.patch` that you can apply:

```bash
cd ~/Projects/lcd4linux
patch -p1 < ~/Projects/linux4lcd-esp-c3/lcd4linux-esp32oled.patch
./bootstrap
./configure --with-esp32oled
make -j$(nproc)
sudo make install
```

## Troubleshooting

### Configure fails with "command not found"
```bash
# Regenerate autotools files
autoreconf -fi
```

### Missing dependencies error
```bash
# Check what's missing
./configure --with-esp32oled

# Install the specific missing library
# Example for libgd:
sudo apt-get install libgd-dev
```

### Driver not showing in list
```bash
# Check if it was compiled
grep ESP32OLED config.h
# Should show: #define WITH_ESP32OLED 1

# Check object file was created
ls -la drv_ESP32OLED.o
```

### Serial port permission denied
```bash
# Add user to dialout group
sudo usermod -a -G dialout $USER

# Or run with sudo (not recommended for production)
sudo lcd4linux -F -f /etc/lcd4linux.conf
```

### "Device not responding" error
- Ensure ESP32-C3 is connected and has the Arduino sketch uploaded
- Verify the port in config file matches: `ls /dev/ttyACM*`
- Test manually: `echo "PING" > /dev/ttyACM0`
- Check baud rate matches (115200)

## Configuration

After successful compilation, use the native configuration file:

```bash
# Copy native config
sudo cp lcd4linux-native.conf /etc/lcd4linux.conf

# Edit port if needed
sudo nano /etc/lcd4linux.conf

# Test run
sudo lcd4linux -F -f /etc/lcd4linux.conf

# Run as daemon
sudo systemctl enable lcd4linux
sudo systemctl start lcd4linux
```

## Performance Notes

Native driver vs Python bridge:
- ✅ **Lower CPU usage** - no Python interpreter overhead
- ✅ **Better integration** - native lcd4linux widgets and plugins
- ✅ **Standard daemon** - uses system lcd4linux service
- ⚠️ **More complex setup** - requires compilation
- ⚠️ **Less flexible** - requires recompilation for changes

## Uninstall

```bash
cd ~/Projects/lcd4linux
sudo make uninstall

# Or manually:
sudo rm /usr/local/bin/lcd4linux
sudo rm /usr/local/lib/liblcd4linux*
```

## Resources

- LCD4Linux GitHub: https://github.com/TangoCash/lcd4linux
- LCD4Linux Wiki: https://ssl.bulix.org/projects/lcd4linux/
- Driver API: https://ssl.bulix.org/projects/lcd4linux/wiki/DriverAPI
