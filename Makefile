#######################################
# mkpwd Makefile                      #
# (c) 2025 Nathan Gill                #
# https://github.com/OldUser101/mkpwd #
#######################################

## VARIABLES ##
SRC_DIR_MKPWD := src
BIN_DIR := bin
PKG_DIR := pkg
DOC_DIR := doc

DEBIAN_PKG_DIR := $(PKG_DIR)/deb
DEBIAN_CONF_DIR := $(DEBIAN_PKG_DIR)/DEBIAN
DEBIAN_BIN_DIR := $(DEBIAN_PKG_DIR)/usr/bin
DEBIAN_MAN_DIR := $(DEBIAN_PKG_DIR)/usr/share/man/man1
DEBIAN_DOC_DIR := $(DEBIAN_PKG_DIR)/usr/share/doc/mkpwd

MAN_MKPWD := mkpwd.1
LOG_MKPWD := changelog
DEB_PACKAGE := mkpwd.deb

TARGET_MKPWD := $(BIN_DIR)/mkpwd
PKG_TARGET_MKPWD := $(DEBIAN_BIN_DIR)/mkpwd

EXEC_FILES := $(PKG_TARGET_MKPWD)
RES_FILES := $(DEBIAN_DOC_DIR)/copyright $(DEBIAN_MAN_DIR)/$(MAN_MKPWD).gz $(DEBIAN_DOC_DIR)/$(LOG_MKPWD).gz

SRC_MKPWD := $(wildcard $(SRC_DIR_MKPWD)/*.c)
OBJ_MKPWD := $(patsubst %.c,%.o,$(SRC_MKPWD))

CC := gcc
CFLAGS := -Wall -g

all: $(TARGET_MKPWD)

## EXECUTABLES ##
$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

$(TARGET_MKPWD): $(OBJ_MKPWD) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(OBJ_MKPWD)
	strip --strip-unneeded $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

## PACKAGES ##
$(DEBIAN_BIN_DIR) $(DEBIAN_MAN_DIR) $(DEBIAN_DOC_DIR):
	@mkdir -p $@
	
$(DEBIAN_CONF_DIR):
	cp -r debian $(DEBIAN_CONF_DIR)

package: $(TARGET_MKPWD) $(DEBIAN_BIN_DIR) $(DEBIAN_CONF_DIR) $(DEBIAN_MAN_DIR) $(DEBIAN_DOC_DIR)
	cp $(TARGET_MKPWD) $(PKG_TARGET_MKPWD)
	gzip --best -n -c $(DOC_DIR)/$(MAN_MKPWD) > $(DEBIAN_MAN_DIR)/$(MAN_MKPWD).gz
	gzip --best -n -c $(DOC_DIR)/$(LOG_MKPWD) > $(DEBIAN_DOC_DIR)/$(LOG_MKPWD).gz
	cp LICENSE $(DEBIAN_DOC_DIR)/copyright
	find $(DEBIAN_PKG_DIR) -type d -exec chmod 755 {} +
	chmod 755 $(EXEC_FILES)
	chmod 644 $(RES_FILES)
	sudo chown -R root:root $(DEBIAN_PKG_DIR)
	dpkg-deb --build $(DEBIAN_PKG_DIR) $(PKG_DIR)/$(DEB_PACKAGE)

## CLEANUP ##
clean:
	rm -f $(OBJ_MKPWD) $(TARGET_MKPWD)

distclean: clean
	rm -rf $(PKG_DIR)

.PHONY: all clean

