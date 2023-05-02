# SimpleMBRUtility
A tool only for malware testers or any tech enthusiasts made in C++ for protecting the MBR of a computer
# FixMBRApp
This Program asks you to install it when first run if the MBR is valid if it's not it asks if you are in a PE(Preinstallation Environment),
and this will also work if you are not in a PE and haven't restarted the computer after the MBR got damaged, it'll ask you if you have bootloader backup
if yes it'll help you restore it if not it'll help you recover the MBR by the own tool itself(STILL IN DEVELOPMENT),
and hopping back when the MBR is valid you might install the tool which will backup the MBR to system32 folder and also create a Task Scheduler task to
bypass UAC and it then monitors the MBR if it's damaged or not, if it is it repairs it and prompts the user(this goes until the user logs of restarts or does anything else) and it starts again when any user logs in..
# FixMBRApp(Uninstaller)
This is the uninstaller, you need to run this with /uninstall switch to uninstall the tool
