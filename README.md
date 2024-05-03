# WiFi Checking
Checks connectivity for multiple WiFi networks every x minutes.

## TODO

### SW
- [x] Reading network credentials from csv file
- [x] Checking connectivity for every wi-fi network
- [x] Reporting via WhatsApp message
    - [ ] HTML encode names of bad networks before sending to WA
    - [ ] Don't report network every time it's bad - only when it went from good to bad (and from bad to good)
- [ ] Add disconnect function and call it after successful connects
- [ ] Add one or two retries to check if a problem with some wifi is temporary
- [ ] Logging changes in network statuses (current time needed)

### HW
- [ ] Can this device "steal" power from the router (step down converter 12V->5V)?
- [ ] Can this device reset router via transistor or relay?
