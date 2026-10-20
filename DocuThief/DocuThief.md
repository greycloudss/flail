<p align="left">
  <a href="../readme.md">⬅ Back</a>
</p>

# DocuThief: Non-Functional Lua Document Stealer

## What It Is

DocuThief is a Lua script that tries to steal PDFs and Word docs from a Windows user's Downloads folder and send them to a remote host over TCP. It doesn't work—on purpose. It's part of the Flail repo, a collection of malware written for research and curiosity.

## How It Works

The script shells out to PowerShell via `io.popen` to recursively list `*.pdf` and `*.doc*` files in `%USERPROFILE%\Downloads`. It parses the output, then for each file opens a TCP socket and sends a `FILE <path> <size>` header followed by the file contents in 64KB chunks, with a small random delay between sends (a jitter trick real malware sometimes uses to dodge network detection).

# Why Lua?

Lua is a weird choice for malware, which is exactly the point. Tools like LucidRook have used it for the same reason—EDR products are tuned for PowerShell, JavaScript, and compiled binaries, not a scripting language mostly seen in game mods. It's lightweight and easy to embed, and it flies under a lot of radar.

## The Mechanism
The mechanism is very simple in nature: Go through the user path variable, list all of the files, turn them over to the thievery function (*reportFiles()*), chop it up into chunks, send it off and die. This does not have any extreme persistence mechanisms, it is a mere knave.


<p align="left">
  <a href="../readme.md">⬅ Back</a>
</p>
