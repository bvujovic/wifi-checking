# WiFi Checking
Checks connectivity for multiple WiFi networks every x minutes.

## TODO

### SW
- [x] Reading network credentials from csv file
- [x] Checking connectivity for every wi-fi network
- [x] Reporting via WhatsApp message
    - [x] HTML encode spaces in names of bad networks before sending to WA
    - [x] Don't report network every time it's bad - only when it went from good to bad (and from bad to good)
- [x] Add disconnect function and call it after successful connects
- [ ] Web server page for editing nets.csv (turning on and off checking wifi networks)
- [ ] Add one or two retries to check if a problem with some wifi is temporary
- [ ] Logging changes in network statuses (current time needed)

### HW
- [ ] Can this device "steal" power from the router (step down converter 12V->5V)?
- [ ] Can this device reset router via transistor or relay?
