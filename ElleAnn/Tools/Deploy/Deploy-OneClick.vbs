Set shell = CreateObject("WScript.Shell")
cmd = """" & CreateObject("Scripting.FileSystemObject").GetParentFolderName(WScript.ScriptFullName) & "\Deploy-OneClick.cmd""""
shell.Run cmd, 0, False
