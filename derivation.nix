{ pkgs, stdenv }:
stdenv.mkDerivation {
    name = "librxui";

    src = ./.;

    nativeBuildInputs = [
        pkgs.cmake
        pkgs.pkgconfig
    ];

    buildInputs = [
        pkgs.gtk3
        pkgs.qt5.qtbase
    ];

    installPhase = ''
        mkdir $out
        mv librxui-* rxui-* $out
    '';
}
