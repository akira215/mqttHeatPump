# OTA script to update remotely the device

Because raw UDP does not guarantee packet order or delivery, a production system should use CoAP (such as OpenThread's built-in otCoap APIs) or add sequence numbers and acknowledgments. For quick local testing over your Border Router, run this Python script from your computer


• Preventing 6LoWPAN Fragmentation. IEEE 802.15.4 frames have a 127-byte physical MTU. Keep individual UDP/CoAP payloads around 256 to 512 bytes so 6LoWPAN fragmentation does not overwhelm Thread message buffers.
• App Rollback Protection. Enable CONFIG_BOOTLOADER_APP_ROLLBACK_ENABLE in menuconfig. On boot, verify that your device successfully rejoins the Thread network, then call esp_ota_mark_app_valid_cancel_rollback() so a faulty image automatically reverts to the previous firmware

VScode could load automatically the taks.json using lightweight addon `actboy168.tasks`

Add a file `.vscode/tasks.json`

```json
{
    "version": "2.0.0",
    "inputs": [
        {
            "id": "threadDeviceIpv6",
            "type": "promptString",
            "description": "Enter Target OpenThread Device IPv6 Address",
            "default": "fdde:ad00:beef:0:..."
        }
    ],
    "tasks": [
        {
            "label": "Thread OTA Flash",
            "type": "shell",
            "command": "python3",
            "args": [
                "${workspaceFolder}/scripts/ota_script.py",
                "${input:threadDeviceIpv6}",
                "${workspaceFolder}/build/${workspaceFolderBasename}.bin"
            ],
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "presentation": {
                "reveal": "always",
                "panel": "new"
            }
        }
    ]
}
```