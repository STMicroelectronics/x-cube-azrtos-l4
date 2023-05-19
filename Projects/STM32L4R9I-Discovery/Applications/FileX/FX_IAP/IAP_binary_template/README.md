## <b>IAP_binary_template application description</b>

This is an example Application to be used with the IAP_main. This example is meant to demonstrate the base configuration 
that need to be used in order to successfully deploy an application with the IAP_main bootloader.

This application should be configured to start from an offset into the flash that does not overlap with the IAP_main application memory sections.
Particularly, linker options should be changed to set the **Vector Table** and the **ROM START** both pointing to **APP_ADDRESS**.

This application must be generated as raw binary, this can be achieved by setting the output format of the IDE to **Raw binary**.

### <b>Expacted behaviour:</b>
IAP_binary_template should toggle both LEDs.

### <b>Error behaviour:</b>
On failure, orange LED should toggle.

### <b>Keywords</b>

IAP, binary, template

### <b>Note</b>
This application can be debugged using IAR by going into **Project** menu and click **Attach to Running Target**.

A pre-built binary can be found under BIN directory.
