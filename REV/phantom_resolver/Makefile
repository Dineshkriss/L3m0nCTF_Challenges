CC = gcc
CFLAGS = -O2 -Wall
LDFLAGS = -shared -fPIC

SRC_DIR = src
DIST_DIR = dist

.PHONY: all clean

all: $(DIST_DIR)/libmonitor.so $(DIST_DIR)/server_daemon.template

$(DIST_DIR)/libmonitor.so: $(SRC_DIR)/libmonitor.c
	@mkdir -p $(DIST_DIR)
	$(CC) $(LDFLAGS) $(CFLAGS) $< -o $@
	strip --strip-all $@
	@echo "[+] Built and stripped libmonitor.so"

$(DIST_DIR)/server_daemon.template: $(SRC_DIR)/server_daemon.c $(DIST_DIR)/libmonitor.so
	@mkdir -p $(DIST_DIR)
	$(CC) $(CFLAGS) $< -o server_daemon_temp -L./$(DIST_DIR) -lmonitor -Wl,-rpath,.
	cp server_daemon_temp $@
	@./patch_template.sh
	@rm -f server_daemon_temp
	@echo "[+] Created server_daemon.template"

clean:
	rm -rf $(DIST_DIR)
	rm -f server_daemon_temp
	@echo "[+] Cleaned build artifacts"
