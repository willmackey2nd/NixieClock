# NixieClock
Modded fw for gra & afch NCS314_v3.4 board

Original fw here https://github.com/afch/NixieClock

# Changes
1. Removed the IR control support
2. Removed the intro song
3. Removed the alarm menu
4. Removed the automatic "mode change" and instead keep displaying time
5. If entered in menu / edit mode, it'll revert automatically back to displaying time after a delay
6. Added a (imo) nicer anti-poisoning slot machine effect  
   - Occurs every minute and for a longer period every hour
   - Scrolls numbers faster and stops one at a time, in an order depending which ever reaches the "target number" first
8. Added a "sleep time" function
   - New menu after temperature display mode
   - On/off, sleep start time (HH:mm) and end time (HH:mm)
   - Overrides motion detection
9. Added a motion detection auto sleep
   - Used RCWL0516 radar with +5VDC supply and signal connected to D8 pin. Has a quite suitable 1 room coverage.
   - Tubes & leds shutdown after set time (1h) if no motion is detected
10. Added a fade in/out animation
   - RGB fades in/out from current color
   - Tubes have a random flicker effect that follows RGB brightness
11. Up/down button hold function fixed
12. Some general tweaking, cleaning & fixing

Slot machine + fade effects:
https://www.youtube.com/watch?v=ue7s6oBZFs8
