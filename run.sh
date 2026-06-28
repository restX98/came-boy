#!/usr/bin/env bash

# TODO: probably temporary, I don't really like to depend on tmux for this, but it's convenient for now
set -euo pipefail

BINARY="./build/came-boy"
ROM="${1:-}"

if [ -z "$ROM" ]; then
    echo "Usage: $0 <rom.gb>"
    exit 1
fi

if [ ! -f "$BINARY" ]; then
    echo "Binary not found: $BINARY — run 'make' first"
    exit 1
fi

if [ ! -f "$ROM" ]; then
    echo "ROM not found: $ROM"
    exit 1
fi

INTERNAL_LOG=$(mktemp /tmp/came-boy-internal-XXXX)
SERIAL_LOG=$(mktemp /tmp/came-boy-serial-XXXX)

cleanup() {
    rm -f "$INTERNAL_LOG" "$SERIAL_LOG"
}
trap cleanup EXIT

# Layout (SDL video will replace the right pane later):
# ┌─────────────┬─────────────┐
# │  internal   │   serial    │
# │    log      │    out      │
# │             │             │
# └─────────────┴─────────────┘
tmux new-session \; \
    rename-window "came-boy" \; \
    send-keys "tail -f '$INTERNAL_LOG'" Enter \; \
    split-window -h "tail -f '$SERIAL_LOG'" \; \
    select-pane -t 0 \; \
    new-window \; \
    send-keys "$BINARY '$ROM' >'$SERIAL_LOG' 2>'$INTERNAL_LOG'" Enter \; \
    select-window -t 0
