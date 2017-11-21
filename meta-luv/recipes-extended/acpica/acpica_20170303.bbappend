# acpixtract*.bb is a subset of this recipe, so RREPLACE it
PROVIDES += "acpixtract"
RPROVIDES_${PN} += "acpixtract"
RREPLACES_${PN} += "acpixtract"
RCONFLIGHTS_${PN} += "acpixtract"
