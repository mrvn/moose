#!/bin/sh

set -e

srcpath() {
    DIR="$1"
    SRC="$2"

    if [ "$DIR" = "." ]; then
	DIR=""
    fi

    case "$SRC" in
	(/*) # absolute path
	    printf "%s" "${SRC#/*}"
	    ;;
	(../*) # relative path
	    objpath "$(dirname "$DIR")" "${SRC#../*}"
	    ;;
	(*) # local file
	    printf "%s%s" "$DIR" "$SRC"
	    ;;
    esac
}

objname() {
    DIR="$1"
    SRC="$DIR$(basename "$2")"

    case "$SRC" in
	(*.S)
	    printf "%s.o" "$SRC"
	    ;;
	(*.c|*.cc)
	    printf "%s.lto" "$SRC"
	    ;;
	(*.o|*.lto) # for _tmp-*.*
	    printf "%s" "$SRC"
	    ;;
	(*)
	    echo >&2 "Error: Unknown source type '$SRC'"
	    exit 1
	    ;;
    esac
}

SRCDIR="$1"
DST="$2"
SRC="$(printf "%s" "$DST" | sed -e 's,__,/,g' -e 's/.mk$/.obj/' -e 's,^_mk/,,')"

printf "%s: %s\n" "$DST" "$SRC" >"$DST".tmp

DIR="$(dirname "$SRC")/"
if [ "$DIR" = "./" ]; then
    DIR=""
fi

printf "%s_tmp-y.lto: override LD_TXT = \n" "${DIR}" >>"$DST".tmp
printf "%s_tmp-y.lto: override LOCAL_LD = \$(LD)\n" "$DIR" >>"$DST".tmp
printf "%s_tmp-y.lto: _empty.c.lto\n" "$DIR" >>"$DST".tmp
printf "%s_tmp-y.o: override LD_TXT = \n" "$DIR" >>"$DST".tmp
printf "%s_tmp-y.o: override LOCAL_LD = \$(LD)\n" "$DIR" >>"$DST".tmp
printf "%s_tmp-y.o: _empty.c.o\n" "$DIR" >>"$DST".tmp

TGT="lto"
while read TYPE ARG REST; do
    case "$TYPE" in
	("DOLTO")
	    TGT="o"
	    printf "%s_tmp-y.o: override LD_TXT = [LTO]\n" "$DIR" >>"$DST".tmp
	    printf "%s_tmp-y.o: override LOCAL_LD = \$(LD_NO_LTO)\n" "$DIR" >>"$DST".tmp
	    ;;
	(LDSCRIPT)
	    TGT="o"
	    printf "%s_tmp-y.o: override LD_TXT = [LTO]\n" "$DIR" >>"$DST".tmp
	    printf "%s_tmp-y.o: override LOCAL_LD = \$(LD_NO_LTO) -T %sldscript\n" "$DIR" "$DIR" >>"$DST".tmp
	    printf "%s_tmp-y.o: | %sldscript\n" "$DIR" "$DIR" >>"$DST".tmp
	    [ -d "$DIR" ] || mkdir -p "$DIR"
	    cat > "${DIR}ldscript" <<EOF
SECTIONS
{
    .text$ARG : {
        *(.text)
        *(.text.*)
    }
    .rodata$ARG : {
        *(.rodata)
        *(.rodata.*)
    }
    .init_array$ARG : {
        *(SORT_BY_INIT_PRIORITY(.init_array))
        *(SORT_BY_INIT_PRIORITY(.init_array.*))
    }
    .data$ARG : {
        *(.data)
        *(.data.*)
    }
    .bss$ARG : {
        *(.bss)
        *(.bss.*)
    }
}
EOF
	    ;;
	("ASFLAGS"|"CFLAGS"|"CXXFLAGS")
	    FULLSRC="$(srcpath "$DIR" "$ARG")"
	    OBJ="$(objname "$DIR" "$ARG")"
	    printf "%s: override %s %s\n" "$OBJ" "$TYPE" "$REST"
	    printf "%s: override %s_TXT = [%s %s]\n" "$OBJ" "$TYPE" "$TYPE" "$REST"
	    ;;
	("FLAGS")
	    FULLSRC="$(srcpath "$DIR" "$ARG")"
	    OBJ="$(objname "$DIR" "$ARG")"
	    for TYPE in ASFLAGS CFLAGS CXXFLAGS; do
		printf "%s: override %s %s\n" "$OBJ" "$TYPE" "$REST"
		printf "%s: override %s_TXT = [%s %s]\n" "$OBJ" "$TYPE" "$TYPE" "$REST"
	    done
	    ;;
	("SRC")
	    for SRC in $REST; do
		FULLSRC="$(srcpath "$DIR" "$SRC")"
		OBJ="$(objname "$DIR" "$SRC")"
		printf "%s: %s\n" "$OBJ" "$FULLSRC"
		case "$OBJ" in
		    (*.lto)
			printf "%s_tmp-%s.%s: %s\n" \
			       "$DIR" "$ARG" "$TGT" "$OBJ"
			;;
		    (*.o)
			printf "%s_tmp-%s.o: %s%s\n" \
			       "$DIR" "$ARG" "$OBJ"
			;;
		esac
	    done
	    ;;
	("DIR")
	    LTO="$(for OBJ in $REST; do printf " %s%s/_tmp-y.lto" "$DIR" "$OBJ"; done)"
	    printf "%s_tmp-%s.%s:%s\n" "$DIR" "$ARG" "$TGT" "$LTO"
	    OBJ="$(for OBJ in $REST; do printf " %s%s/_tmp-y.o" "$DIR" "$OBJ"; done)"
	    printf "%s_tmp-%s.o:%s\n" "$DIR" "$ARG" "$OBJ"
	    ;;
	(""|"#"*)
	    : # Empty or comment
	    ;;
	(*)
	    echo >&2 "### unknown type '$TYPE'"
	    exit 1
	    ;;
    esac
done <"$SRCDIR/$SRC" >>"$DST".tmp
mv "$DST".tmp "$DST"
