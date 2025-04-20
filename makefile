# Variables
CC = gcc
SRC_DIR = Software/src
BUILD_DIR = Software/src
TARGET = pam_zkp.so

SRC_FILES = $(SRC_DIR)/PAM_Module.c $(SRC_DIR)/ZKP_verify.c
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRC_FILES))

# Default target
all: $(BUILD_DIR)/$(TARGET)

# Build the shared library
$(BUILD_DIR)/$(TARGET): $(OBJ_FILES)
	@mkdir -p $(BUILD_DIR)
	@echo "🔧 Compiling PAM module..."
	$(CC) -shared -o $@ $(OBJ_FILES) -lpam -lgmp
	@echo "✅ Build complete: $@"

# Rule to compile each .c to .o
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) -fPIC -c $< -o $@

# Install the shared library to /lib/security
install: $(BUILD_DIR)/$(TARGET)
	sudo cp $< /usr/lib/x86_64-linux-gnu/security/
	sudo chown root:root /usr/lib/x86_64-linux-gnu/security/$(TARGET)
	sudo chmod 644 /usr/lib/x86_64-linux-gnu/security/$(TARGET)
	sudo cp Software/test/pamtest /etc/pam.d/
	sudo cp Software/test/pamtest /etc/pam.d/
	@echo "✅ Installed to /usr/lib/x86_64-linux-gnu/security/"
test:
	gcc Software/test/test_module.c -o Software/test/test_module -lpam -lpam_misc
	sudo Software/test/test_module

# Remove built files
clean:
	rm -rf $(BUILD_DIR)
	@echo "🧹 Cleaned build artifacts"

# Uninstall from system directory
uninstall:
	sudo rm -f /usr/lib/x86_64-linux-gnu/security/$(TARGET)
	sudo rm -f /usr/lib/security/$(TARGET)
	@echo "🗑️ Uninstalled from system security directories"
