find -maxdepth 3 -name "*.cpp" -or -name "*.hpp" | xargs grep TODO | awk -F'//|:|' '{printf "%50s %s\n",$1,$3}' | grep TODO --color=always
