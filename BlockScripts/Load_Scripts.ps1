$header = "BlockScripts\Block_Scripts.h"
$source = "BlockScripts\Block_Scripts.cpp"

if (Test-Path $header) {
    Clear-Content $header
}
else {
    New-Item $header -ItemType file
}

if (Test-Path $source) {
    Clear-Content $source
}
else {
    New-Item $source -ItemType file
}

Add-Content $header ("#pragma once`n")

Add-Content $header ("#include <map>")
Add-Content $header ("#include <string>")
Add-Content $header ("#include <functional>`n")

Add-Content $header ("extern std::map<std::string, std::function<void()>> BlockRightClick;")
Add-Content $header ("extern std::map<std::string, std::function<void()>> BlockUpdate;")
Add-Content $header ("extern std::map<std::string, std::function<void()>> BlockClose;`n")

Add-Content $header ("void Init_Block_Scripts();")

Add-Content $source ("#include `"Block_Scripts.h`"`n")

Get-ChildItem "BlockScripts" -Filter *.h |
Foreach-Object {
    if ($_.name -ne "Block_Scripts.h") {
        Add-Content $source ('#include "' + $_.name + '"')
    }
}

Add-Content $source ("`nstd::map<std::string, std::function<void()>> BlockRightClick = {")

Get-ChildItem "BlockScripts" -Filter *.h |
Foreach-Object {
    if ($_.name -ne "Block_Scripts.h") {
        Add-Content $source ("`t{`"" + $_.basename + "`", " + $_.basename + "::Right_Click" + "},")
    }
}

Add-Content $source ("};")

Add-Content $source ("`nstd::map<std::string, std::function<void()>> BlockUpdate = {")

Get-ChildItem "BlockScripts" -Filter *.h |
Foreach-Object {
    if ($_.name -ne "Block_Scripts.h") {
        Add-Content $source ("`t{`"" + $_.basename + "`", " + $_.basename + "::Update" + "},")
    }
}

Add-Content $source ("};")

Add-Content $source ("`nstd::map<std::string, std::function<void()>> BlockClose = {")

Get-ChildItem "BlockScripts" -Filter *.h |
Foreach-Object {
    if ($_.name -ne "Block_Scripts.h") {
        Add-Content $source ("`t{`"" + $_.basename + "`", " + $_.basename + "::Close" + "},")
    }
}

Add-Content $source ("};`n")

Add-Content $source ("void Init_Block_Scripts() {")
Get-ChildItem "BlockScripts" -Filter *.h |
Foreach-Object {
    if ($_.name -ne "Block_Scripts.h") {
        Add-Content $source ("`t" + $_.basename + "::Init();")
    }
}

Add-Content $source ("};")