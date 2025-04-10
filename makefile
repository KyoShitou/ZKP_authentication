install:
	sudo cp ./Software/src/pam_zkp.so /lib/security/
	sudo chown root:root /lib/security/pam_zkp.so
	sudo chmod 644 /lib/security/pam_zkp.so
# Variables
SRC_DIR := ./Software/src
BUILD_DIR := $(SRC_DIR)
TARGET := pam_zkp.so
OBJ := PAM_Module.o

# Default target
all: build

# Build the shared library
build:
	@echo "🔧 Compiling PAM module..."
	$(CC) -fPIC -c $(SRC_DIR)/PAM_Module.c -o $(BUILD_DIR)/$(OBJ)
	$(CC) -shared -o $(BUILD_DIR)/$(TARGET) $(BUILD_DIR)/$(OBJ) -lpam
	@echo "✅ Build complete: $(BUILD_DIR)/$(TARGET)"

# Install the shared library to /lib/security
install: build
	sudo mkdir -p /lib/security
	sudo cp $(BUILD_DIR)/$(TARGET) /lib/security/
	sudo chown root:root /lib/security/$(TARGET)
	sudo chmod 644 /lib/security/$(TARGET)
	@echo "✅ Installed to /lib/security/"

# Remove built files
clean:
	rm -f $(BUILD_DIR)/$(OBJ) $(BUILD_DIR)/$(TARGET)
	@echo "🧹 Cleaned build artifacts"

# Uninstall from system directory
uninstall:
	sudo rm -f /lib/security/$(TARGET)
	@echo "🗑️ Uninstalled from /lib/security/"
