# install:
# 	sudo cp ./Software/src/pam_zkp.so /usr/lib/x86_64-linux-gnu/security/
# 	sudo chown root:root /usr/lib/x86_64-linux-gnu/security/pam_zkp.so
# 	sudo chmod 644 /usr/lib/x86_64-linux-gnu/security/pam_zkp.so
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
	$(CC) -shared -o $(BUILD_DIR)/$(TARGET) $(BUILD_DIR)/$(OBJ) -lpam -lgmp
	@echo "✅ Build complete: $(BUILD_DIR)/$(TARGET)"

# Install the shared library to /lib/security
install:
	sudo cp $(BUILD_DIR)/$(TARGET) /usr/lib/x86_64-linux-gnu/security/
	sudo chown root:root /usr/lib/x86_64-linux-gnu/security/$(TARGET)
	sudo chmod 644 /usr/lib/x86_64-linux-gnu/security/$(TARGET)

	sudo cp $(BUILD_DIR)/$(TARGET) /usr/lib/security/
	sudo chown root:root /usr/lib/security/$(TARGET)
	sudo chmod 644 /usr/lib/security/$(TARGET)
	@echo "✅ Installed to /usr/lib/security/"

# Remove built files
clean:
	rm -f $(BUILD_DIR)/$(OBJ) $(BUILD_DIR)/$(TARGET)
	@echo "🧹 Cleaned build artifacts"

# Uninstall from system directory
uninstall:
	sudo rm -f /lib/security/$(TARGET)
	@echo "🗑️ Uninstalled from /lib/security/"
