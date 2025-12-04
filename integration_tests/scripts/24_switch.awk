# Test: switch statement (gawk extension)
{
    switch ($3) {
        case "INFO":
            type = "[I]"
            break
        case "WARNING":
            type = "[W]"
            break
        case "ERROR":
            type = "[E]"
            break
        case "DEBUG":
            type = "[D]"
            break
        default:
            type = "[?]"
    }
    print type, $0
}
