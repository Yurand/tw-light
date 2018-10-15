import traceback
import subprocess
import os
import re

DAT_PATH = "../Release/dat/dat.exe"

DITTY = {
# sc1, sc2
"shpandgu.dat": "ANDDITTY_WAV",
"shparisk.dat": "ARIDITTY_WAV",
"shpchebr.dat": "CHEDITTY_WAV",
"shpearcr.dat": "HUMDITTY_WAV",
"shpilwav.dat": "ILWDITTY_WAV",
"shpkzedr.dat": "URQDITTY_WAV",
"shpmmrxf.dat": "MMRDITTY_WAV",
"shpmycpo.dat": "MYCDITTY_WAV",
"shpshosc.dat": "SHODITTY_WAV",
"shpspael.dat": "SPADITTY_WAV",
"shpsyrpe.dat": "SYRDITTY_WAV",
"shpumgdr.dat": "UMGDITTY_WAV",
"shpvuxin.dat": "VUXDITTY_WAV",
"shpyehte.dat": "YEHDITTY_WAV",
"shpchmav.dat": "CHMDITTY_WAV",
"shpdruma.dat": "DRUDITTY_WAV",
"shpkohma.dat": "BLADITTY_WAV",
"shpmeltr.dat": "MELDITTY_WAV",
"shporzne.dat": "ORZDITTY_WAV",
"shppkufu.dat": "PKUDITTY_WAV",
"shpslypr.dat": "SLYDITTY_WAV",
"shpsupbl.dat": "SUPDITTY_WAV",
"shpthrto.dat": "THRDITTY_WAV",
"shputwju.dat": "UTWDITTY_WAV",
"shpzfpst.dat": "ZOQDITTY_WAV",
# TW special
"shpalabc.dat": "ALADITTY_WAV",
"shpforsh.dat": "FORDITTY_WAV",
"shpzeksh.dat": "FORDITTY_WAV",

"shptauar.dat": "TAUDITTY_WAV",
"shptaubo.dat": "TAUDITTY_WAV",
"shptauda.dat": "TAUDITTY_WAV",
"shptauem.dat": "TAUDITTY_WAV",
"shptaufi.dat": "TAUDITTY_WAV",
"shptaugl.dat": "TAUDITTY_WAV",
"shptauhu.dat": "TAUDITTY_WAV",
"shptaule.dat": "TAUDITTY_WAV",
"shptaumc.dat": "TAUDITTY_WAV",
"shptaume.dat": "TAUDITTY_WAV",
"shptausl.dat": "TAUDITTY_WAV",
"shptaust.dat": "TAUDITTY_WAV",
"shptauto.dat": "TAUDITTY_WAV",
"shptautu.dat": "TAUDITTY_WAV",

# human extra
"shpearc2.dat": "HUMDITTY_WAV",
"shpearc3.dat": "HUMDITTY_WAV",
"shpstaba.dat": "HUMDITTY_WAV",

# Arilou extra
"shparitr.dat": "ARIDITTY_WAV",

"shpchmba.dat": "CHMDITTY_WAV",

"shpilwsp.dat": "ILWDITTY_WAV",
}

def run_cmd(argv, contents=None):
    errContents = ""
    try:
        process = subprocess.Popen(argv, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        output, errContents = process.communicate(contents)
        exit_code = process.wait()
        resultunicode = output.decode("utf-8", "replace")
        if not resultunicode:
            return None
        return resultunicode
    except:
        raise IOError("Error using tika\n%s\n'%s'"%(traceback.format_exc(),errContents))

def validate_ship_data():
    valid = True
    SHIP_DATA = [x for x in os.listdir("../data") if x.endswith(".dat") and x.startswith("shp")]
    for ship in SHIP_DATA:
        path = os.path.join("../data", ship)
        output = run_cmd([DAT_PATH, "-l", path])
        if output.find("DATA - SHIP_DAT") == -1:
            print ship + "no ini in datafile"
            valid = False
    return valid

def set_ditties():
    valid = True
    SHIP_DATA = [x for x in os.listdir("../data") if x.endswith(".dat") and x.startswith("shp")]
    for ship in SHIP_DATA:
        print ship
        path = os.path.join("../data", ship)
        output = run_cmd([DAT_PATH, "-l", path])
        if output.find("DATA - SHIP_DAT") == -1:
            print ship + "no ini in datafile"
            continue
        if output.find("VICTORY_WAV") != -1:
            run_cmd([DAT_PATH, "-d", path, "VICTORY_WAV"])
        if output.find("VICTORYM_MOD") != -1:
            run_cmd([DAT_PATH, "-d", path, "VICTORYM_MOD"])
        if os.path.exists("SHIP_DAT"):
            os.unlink("SHIP_DAT")
        output = run_cmd([DAT_PATH, "-e", path, "SHIP_DAT"])
        data = open("SHIP_DAT", "rb").read()
        m = re.search("(VictoryDitty\s*=\s*)(.+)", data)
        if m:
            data = data.replace(m.group(), "VictoryDitty     = " + DITTY.get(ship, "VOIDDITTY_WAV"))
        else:
            data = data.replace("[Objects]", "[Objects]\r\nVictoryDitty     = " + DITTY.get(ship, "VOIDDITTY_WAV"))
        data = open("SHIP_DAT", "wb").write(data)
        output = run_cmd([DAT_PATH, "-a", path, "SHIP_DAT"])
        output = run_cmd([DAT_PATH, "-c0", "-s1", path])

def compute():
    print "validating..."
    valid = validate_ship_data()
    print valid
    if not valid:
        print "Ship data is not right ... exiting"
        return
    set_ditties()

compute()


