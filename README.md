# Namco System256 Regional signature changer

this application overrides the iLinkID of the ps2 nvram on the system256.

This signature seems to be used only by the taiko drum games

## How to use

Run the program in any way you can, make sure to run the program you really need

## Regional Signatures

Name  | Actual Signature | Games that look for it
:---- | :--------------: | --------------------:
ASIA5 | `41 46 53 2F 1E FD 0F E0` | `NM00045`, `NM00053`
ASIA4 | `32 1F C7 FA D6 EE F0 1C` | `NM00046` and `NM00054`
JAPAN | `FF FF FF FF FF FF FF FF` | The rest of the taiko drum games

## Warning
It's strongly recommended to not abuse the usage of this program

if you have several region variants of taiko drum games, and a single system256 unit, I recommend you to do this:
- change the unit to JAPAN region
- Apply the regional bypass hack to your non japanese taiko drum units (for that, refer to the [DrumTools Repository](https://github.com/PS2Homebrew-arcade/DrumTools) and read about the regional hack and the python script to inject data into IRXARC.BIN from the game dongle)