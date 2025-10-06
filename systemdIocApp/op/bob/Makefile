# Makefile for CSS Phoebus .bob screens

# Default target
all: help

# Help target
help:
	@echo "CSS Phoebus Systemd Service Control Screens"
	@echo "=========================================="
	@echo ""
	@echo "Available targets:"
	@echo "  help              - Show this help message"
	@echo "  list              - List available .bob files"
	@echo "  generate          - Generate example service screens"
	@echo "  clean             - Remove generated files"
	@echo "  install           - Install screens to CSS Phoebus directory"
	@echo ""
	@echo "Manual screen generation:"
	@echo "  ./generate_service_screen.sh <service_name> <pv_prefix> <pv_suffix>"
	@echo "  Example: ./generate_service_screen.sh apache2.service web: server:"
	@echo ""
	@echo "Files:"
	@echo "  systemd_service.bob     - Generic systemd service control screen"
	@echo "  README.md               - Documentation for using the screens"
	@echo "  examples.md             - Example configurations"
	@echo "  generate_service_screen.sh - Script to generate service-specific screens"

# List available .bob files
list:
	@echo "Available .bob files:"
	@ls -la *.bob 2>/dev/null || echo "No .bob files found"

# Generate example service screens
generate:
	@echo "Generating example service screens..."
	@./generate_service_screen.sh apache2.service web: server:
	@./generate_service_screen.sh ssh.service system: ssh:
	@./generate_service_screen.sh postgresql.service db: service:
	@echo "Generated example screens:"
	@ls -la *_service.bob

# Clean generated files
clean:
	@echo "Cleaning generated files..."
	@rm -f *_service.bob
	@echo "Cleaned generated service screens"

# Install screens to CSS Phoebus directory (if available)
install:
	@echo "Installing screens to CSS Phoebus..."
	@if [ -d "$$HOME/.css-phoebus/op/bob" ]; then \
		cp *.bob $$HOME/.css-phoebus/op/bob/; \
		echo "Installed to $$HOME/.css-phoebus/op/bob/"; \
	elif [ -d "/opt/css-phoebus/op/bob" ]; then \
		sudo cp *.bob /opt/css-phoebus/op/bob/; \
		echo "Installed to /opt/css-phoebus/op/bob/"; \
	else \
		echo "CSS Phoebus directory not found. Please install manually."; \
		echo "Copy *.bob files to your CSS Phoebus op/bob directory."; \
	fi

# Validate .bob files
validate:
	@echo "Validating .bob files..."
	@for file in *.bob; do \
		if [ -f "$$file" ]; then \
			echo -n "Validating $$file... "; \
			if xmllint --noout "$$file" 2>/dev/null; then \
				echo "OK"; \
			else \
				echo "FAILED"; \
			fi; \
		fi; \
	done

# Show PV information for a screen
pvs:
	@echo "PVs used in systemd_service.bob:"
	@echo "  \$$(P)\$$(R)ServiceName - Service name display"
	@echo "  \$$(P)\$$(R)Status      - Service status (running, stopped, etc.)"
	@echo "  \$$(P)\$$(R)Start       - Start/Stop control (1=start, 0=stop)"
	@echo "  \$$(P)\$$(R)ResetFailed - Reset failed state (1=reset)"
	@echo ""
	@echo "Example with P=web: and R=server::"
	@echo "  web:server:ServiceName"
	@echo "  web:server:Status"
	@echo "  web:server:Start"
	@echo "  web:server:ResetFailed"

.PHONY: all help list generate clean install validate pvs
