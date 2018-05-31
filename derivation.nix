{ pkgs, lib, stdenv }:
stdenv.mkDerivation rec {
    name = "librxui";

    src = ./.;

    nativeBuildInputs = with pkgs; [
        cmake
        # pkgconfig
        (pkgconfig.override { vanilla = true; })
    ];

    buildInputs = with pkgs; [
        enlightenment.efl

        gtk3
        pcre
        mesa_noglu
        xorg.libpthreadstubs
        xorg.libXdmcp
        epoxy
        at-spi2-core

        qt5.qtbase
    ];

    runtimeInputs = with pkgs; [
        curl
    ];

    shellHook = ''
        export LD_LIBRARY_PATH=''${LD_LIBRARY_PATH:+$LD_LIBRARY_PATH:}${lib.makeLibraryPath runtimeInputs}
    '';

    installPhase = ''
        mkdir $out
        mv librxui-* rxui-* $out
    '';
}
