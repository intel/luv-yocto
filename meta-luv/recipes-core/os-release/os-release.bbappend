OS_RELEASE_FIELDS = "ID ID_LIKE NAME VERSION VERSION_ID PRETTY_NAME BUILD_ID"

BUILD_ID = "122122"

python do_compile () {
    import shutil
    with open(d.expand('${B}/os-release'), 'w') as f:
        for field in d.getVar('OS_RELEASE_FIELDS', True).split():
            value = d.getVar(field, True)
            if value and field == 'VERSION_ID':
                value = sanitise_version(value)
            if value and field == 'BUILD_ID':
                f.write('{0}={1}\n'.format(field, value))
            else:
                f.write('{0}="{1}"\n'.format(field, value))
}
