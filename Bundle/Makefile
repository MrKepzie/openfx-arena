PLUGINNAME = Arena

PLUGINOBJECTS = \
    lodepng.o \
    TextFX.o \
    ReadCDR.o \
    ReadSVG.o \
    ReadKrita.o \
    OpenRaster.o

ifneq ($(LICENSE),COMMERCIAL)
PLUGINOBJECTS += \
    ReadPDF.o
endif

ifneq ($(LEGACYIM),1)
PLUGINOBJECTS += \
    Sketch.o \
    Polaroid.o \
    Edges.o
endif

PLUGINOBJECTS += \
    Texture.o \
    Tile.o \
    Reflection.o \
    Arc.o \
    Polar.o \
    Roll.o \
    Wave.o \
    Swirl.o \
    Implode.o \
    Charcoal.o \
    Oilpaint.o \
    ReadPSD.o \
    Modulate.o \
    ReadMisc.o \
    Text.o \
    MagickPlugin.o \
    ofxsOGLTextRenderer.o \
    ofxsOGLFontData.o \
    ofxsRectangleInteract.o \
    ofxsTransform3x3.o \
    ofxsTransformInteract.o \
    ofxsShutter.o \
    ofxsMultiPlane.o \
    GenericReader.o \
    GenericOCIO.o \
    SequenceParsing.o

RESOURCES = \
    ../Extra/net.fxarena.openfx.Text.png \
    ../Extra/net.fxarena.openfx.Text.svg \
    ../Extra/fr.inria.openfx.ReadCDR.png \
    ../Extra/fr.inria.openfx.ReadCDR.svg \
    ../Extra/net.fxarena.openfx.ReadSVG.png \
    ../Extra/net.fxarena.openfx.ReadSVG.svg \
    ../Extra/fr.inria.openfx.ReadKrita.png \
    ../Extra/fr.inria.openfx.ReadKrita.svg \
    ../Extra/fr.inria.openfx.OpenRaster.png \
    ../Extra/fr.inria.openfx.OpenRaster.svg

ifneq ($(LICENSE),COMMERCIAL)
RESOURCES += \
    ../Extra/fr.inria.openfx.ReadPDF.png \
    ../Extra/fr.inria.openfx.ReadPDF.svg
endif

RESOURCES += \
    ../Magick/Extra.png \
    ../Magick/Extra.svg \
    ../Magick/net.fxarena.openfx.Tile.png \
    ../Magick/net.fxarena.openfx.Tile.svg \
    ../Magick/net.fxarena.openfx.Reflection.png \
    ../Magick/net.fxarena.openfx.Reflection.svg \
    ../Magick/net.fxarena.openfx.Arc.png \
    ../Magick/net.fxarena.openfx.Arc.svg \
    ../Magick/net.fxarena.openfx.Polar.png \
    ../Magick/net.fxarena.openfx.Polar.svg \
    ../Magick/Roll/net.fxarena.openfx.Roll.png \
    ../Magick/Roll/net.fxarena.openfx.Roll.svg \
    ../Magick/Wave/net.fxarena.openfx.Wave.png \
    ../Magick/Wave/net.fxarena.openfx.Wave.svg \
    ../Magick/Swirl/net.fxarena.openfx.Swirl.png \
    ../Magick/Swirl/net.fxarena.openfx.Swirl.svg \
    ../Magick/net.fxarena.openfx.Implode.png \
    ../Magick/net.fxarena.openfx.Implode.svg \
    ../Magick/net.fxarena.openfx.Charcoal.png \
    ../Magick/net.fxarena.openfx.Charcoal.svg \
    ../Magick/net.fxarena.openfx.Oilpaint.png \
    ../Magick/net.fxarena.openfx.Oilpaint.svg \
    ../Magick/net.fxarena.openfx.ReadPSD.png \
    ../Magick/net.fxarena.openfx.ReadPSD.svg \
    ../Magick/net.fxarena.openfx.Modulate.png \
    ../Magick/net.fxarena.openfx.Modulate.svg \
    ../Magick/fr.inria.openfx.ReadMisc.png \
    ../Magick/fr.inria.openfx.ReadMisc.svg \
    ../Magick/net.fxarena.openfx.Texture.png \
    ../Magick/net.fxarena.openfx.Texture.svg

    #../Magick/net.fxarena.openfx.Text.png \
    #../Magick/net.fxarena.openfx.Text.svg

ifneq ($(LEGACYIM),1)
RESOURCES += \
    ../Magick/net.fxarena.openfx.Sketch.png \
    ../Magick/net.fxarena.openfx.Sketch.svg \
    ../Magick/net.fxarena.openfx.Polaroid.png \
    ../Magick/net.fxarena.openfx.Polaroid.svg \
    ../Magick/net.fxarena.openfx.Edges.png \
    ../Magick/net.fxarena.openfx.Edges.svg
endif

SRCDIR = ..

VPATH += \
    $(SRCDIR)/Extra \
    $(SRCDIR)/Magick \
    $(SRCDIR)/Magick/Swirl \
    $(SRCDIR)/Magick/Roll \
    $(SRCDIR)/Magick/Wave

CXXFLAGS += \
    -I$(SRCDIR)/Extra \
    -I$(SRCDIR)/Magick

include $(SRCDIR)/Makefile.master
include $(SRCDIR)/Makefile.io

CXXFLAGS  += \
    $(FCONFIG_CXXFLAGS) \
    $(PANGO_CXXFLAGS) \
    $(SVG_CXXFLAGS) \
    $(CDR_CXXFLAGS) \
    $(XML_CXXFLAGS) \
    $(ZIP_CXXFLAGS) \
    $(MAGICK_CXXFLAGS) \
    $(LCMS_CXXFLAGS) \
    $(GLIB_CXXFLAGS)
LINKFLAGS += \
    $(FCONFIG_LINKFLAGS) \
    $(PANGO_LINKFLAGS) \
    $(SVG_LINKFLAGS) \
    $(CDR_LINKFLAGS) \
    $(XML_LINKFLAGS) \
    $(ZIP_LINKFLAGS) \
    $(MAGICK_LINKFLAGS) \
    $(LCMS_LINKFLAGS) \
    $(GLIB_LINKFLAGS)

CXXFLAGS += -I. -I$(SRCDIR)/Magick

ifneq ($(LICENSE),COMMERCIAL)
CXXFLAGS += $(POPPLER_CXXFLAGS)
LINKFLAGS += $(POPPLER_LINKFLAGS)
endif

ifeq ($(LEGACYIM),1)
CXXFLAGS += -DLEGACYIM -DNOMAGICKSEED
endif

ifeq ($(IM),68)
CXXFLAGS += -DNOMAGICKSEED
endif

# lodepng version, see https://github.com/lvandeve/lodepng
# a70c086077c0eaecbae3845e4da4424de5f43361 is Apr 21, 2016
# c7bb19ad54a53f0d9917da37de7288b1aadf6d0f is Oct 23, 2017
PNGVERSION = c7bb19ad54a53f0d9917da37de7288b1aadf6d0f

lodepng.cpp:
	curl -o $@ https://raw.githubusercontent.com/lvandeve/lodepng/$(PNGVERSION)/lodepng.cpp

lodepng.h:
	curl -o $@ https://raw.githubusercontent.com/lvandeve/lodepng/$(PNGVERSION)/lodepng.h

$(OBJECTPATH)/lodepng.o: lodepng.cpp lodepng.h
$(OBJECTPATH)/ReadKrita.o: ReadKrita.cpp lodepng.h
$(OBJECTPATH)/OpenRaster.o: OpenRaster.cpp lodepng.h
$(OBJECTPATH)/MagickPlugin.o: MagickPlugin.cpp MagickPlugin.h
