// src/extract.c
#include <acs/thermal_image.h>
#include <acs/renderer.h>
#include <acs/palette.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

static void checkAcs(void) {
    ACS_Error err = ACS_getLastError();
    if (err.code) {
        ACS_String* s = ACS_getErrorMessage(err);
        fprintf(stderr, "ACS error: %s | details: %s\n", ACS_String_get(s), ACS_getLastErrorMessage());
        ACS_String_free(s);
        exit(1);
    }
}

static void json_escape(const char* in, char* out, size_t outsz) {
    size_t j = 0;
    for (size_t i=0; in && in[i] && j+2<outsz; ++i) {
        char c = in[i];
        if (c=='"' || c=='\\') { if (j+2<outsz) { out[j++]='\\'; out[j++]=c; } }
        else if (c=='\n') { out[j++]='\\'; out[j++]='n'; }
        else out[j++]=c;
    }
    out[j]=0;
}

static void write_temperature_csv(ACS_ThermalImage* img, const char* csv_path) {
    FILE* f = fopen(csv_path, "w");
    if (!f) { fprintf(stderr, "Cannot open csv for writing: %s\n", csv_path); exit(1); }

    int W = ACS_ThermalImage_getWidth(img);
    int H = ACS_ThermalImage_getHeight(img);
    checkAcs();

    // ⚠️ IMPORTANTE:
    // Abaixo uso uma função *representativa* para obter o valor térmico por pixel.
    // Abra flir_sdk/include/acs/thermal_image.h e confirme a função equivalente:
    // opções comuns em versões do Atlas SDK: 
    // - ACS_ThermalImage_getTemperatureAt(image, x, y)    -> retorna ACS_ThermalValue
    // - ACS_ThermalImage_getPixelTemperature(image, x, y) -> idem
    // Ajuste o nome aqui se o seu header usar outra.
    for (int y=0; y<H; ++y) {
        for (int x=0; x<W; ++x) {
            // --- trecho automático para detectar a função certa ---
            #if defined(ACS_ThermalImage_getObjectTemperatureAt)
                ACS_ThermalValue tv = ACS_ThermalImage_getObjectTemperatureAt(img, x, y);
            #elif defined(ACS_ThermalImage_getTemperatureAt)
                ACS_ThermalValue tv = ACS_ThermalImage_getTemperatureAt(img, x, y);
            #elif defined(ACS_ThermalImage_getPixelTemperature)
                ACS_ThermalValue tv = ACS_ThermalImage_getPixelTemperature(img, x, y);
            #else
                #error "Nenhuma função de leitura de temperatura por pixel encontrada no SDK instalado"
            #endif
            // --------------------------------------------------------
            // Se não existir "getObjectTemperatureAt", cheque por:
            //   ACS_ThermalImage_getTemperatureAt
            //   ACS_ThermalImage_getPixelTemperature
            // ou similar no header.
            double celsius = tv.value; // Geralmente TV.value já vem em °C p/ object temp (confirme no header)
            fprintf(f, (x == W-1) ? "%.6f\n" : "%.6f,", celsius);
        }
    }
    fclose(f);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "{\"ok\":false,\"error\":\"usage: extract <full_path_image>\"}\n");
        return 1;
    }
    const char* path = argv[1];

    ACS_NativeString* ns = ACS_NativeString_createFrom(path);
    ACS_ThermalImage* img = ACS_ThermalImage_alloc();
    ACS_ThermalImage_openFromFile(img, ACS_NativeString_get(ns));
    ACS_NativeString_free(ns);
    checkAcs();

    int W = ACS_ThermalImage_getWidth(img);
    int H = ACS_ThermalImage_getHeight(img);

    const ACS_Palette* pal = ACS_ThermalImage_getPalette(img);
    const char* pal_name = pal ? ACS_Palette_getName(pal) : "";

    ACS_Image_CameraInformation* ci = ACS_ThermalImage_getCameraInformation(img);
    ACS_ThermalParameters* tp = ACS_ThermalImage_getThermalParameters(img);

    char esc[1024];

    // ----- caminho CSV para matriz
    const char* slash = strrchr(path, '/');
    const char* fname = slash ? slash+1 : path;
    char csv_path[512];
    snprintf(csv_path, sizeof(csv_path), "/app/out/%s.csv", fname);

    // cria pasta out (idempotente)
    system("mkdir -p /app/out");

    // grava a matriz
    write_temperature_csv(img, csv_path);

    // ----- coleta medições/anotações
    ACS_Measurements* m = ACS_ThermalImage_getMeasurements(img);
    // Spots
    ACS_ListMeasurementSpot* spots = m ? ACS_Measurements_getAllSpots(m) : NULL;
    size_t n_spots = spots ? ACS_ListMeasurementSpot_size(spots) : 0;

    // Retângulos
    ACS_ListMeasurementRectangle* rects = m ? ACS_Measurements_getAllRectangles(m) : NULL;
    size_t n_rects = rects ? ACS_ListMeasurementRectangle_size(rects) : 0;

    // Elipses
    ACS_ListMeasurementEllipse* elps = m ? ACS_Measurements_getAllEllipses(m) : NULL;
    size_t n_elps = elps ? ACS_ListMeasurementEllipse_size(elps) : 0;

    // Polylines
    ACS_ListMeasurementPolyline* polys = m ? ACS_Measurements_getAllPolylines(m) : NULL;
    size_t n_polys = polys ? ACS_ListMeasurementPolyline_size(polys) : 0;

    // ----- imprime JSON
    printf("{");
    printf("\"ok\":true,");
    // básicos
    json_escape(fname, esc, sizeof esc);
    printf("\"file\":\"%s\",", esc);
    printf("\"width\":%d,\"height\":%d,", W, H);
    json_escape(pal_name ? pal_name : "", esc, sizeof esc);
    printf("\"palette\":\"%s\",", esc);

    // camera info
    printf("\"camera\":{");
    json_escape(ACS_Image_CameraInformation_getModelName(ci), esc, sizeof esc);
    printf("\"model\":\"%s\",", esc);
    json_escape(ACS_Image_CameraInformation_getSerialNumber(ci), esc, sizeof esc);
    printf("\"serial\":\"%s\",", esc);
    json_escape(ACS_Image_CameraInformation_getLens(ci), esc, sizeof esc);
    printf("\"lens\":\"%s\",", esc);
    json_escape(ACS_Image_CameraInformation_getProgramVersion(ci), esc, sizeof esc);
    printf("\"programVersion\":\"%s\"", esc);
    printf("},");

    // thermal params
    printf("\"thermalParameters\":{");
    printf("\"distance\":%.6f,", ACS_ThermalParameters_getObjectDistance(tp));
    printf("\"emissivity\":%.6f,", ACS_ThermalParameters_getObjectEmissivity(tp));
    printf("\"reflected\":%.6f,", ACS_ThermalParameters_getObjectReflectedTemperature(tp).value);
    printf("\"humidity\":%.6f,", ACS_ThermalParameters_getRelativeHumidity(tp));
    printf("\"atmospheric\":%.6f,", ACS_ThermalParameters_getAtmosphericTemperature(tp).value);
    printf("\"transmission\":%.6f", ACS_ThermalParameters_getAtmosphericTransmission(tp));
    printf("},");

    // anotações
    printf("\"measurements\":{");

    // spots
    printf("\"spots\":[");
    for (size_t i=0; i<n_spots; ++i) {
        ACS_MeasurementSpot* s = ACS_ListMeasurementSpot_item(spots, i);
        const ACS_MeasurementShape* sh = ACS_Spot_asMeasurementShape(s);
        ACS_Point p = ACS_MeasurementSpot_getPosition(s);
        ACS_String* label = ACS_MeasurementShape_getLabel(sh);
        json_escape(label ? ACS_String_get(label) : "", esc, sizeof esc);
        printf("{\"id\":%d,\"x\":%d,\"y\":%d,\"label\":\"%s\"}%s",
               ACS_MeasurementShape_getId(sh), p.x, p.y, esc, (i+1<n_spots?",":""));
        ACS_String_free(label);
    }
    printf("],");

    // rectangles
    printf("\"rectangles\":[");
    for (size_t i=0; i<n_rects; ++i) {
        ACS_MeasurementRectangle* r = ACS_ListMeasurementRectangle_item(rects, i);
        const ACS_MeasurementShape* sh = ACS_MeasurementRectangle_asMeasurementShape(r);
        ACS_Rect rc = ACS_MeasurementRectangle_getRect(r);
        ACS_String* label = ACS_MeasurementShape_getLabel(sh);
        json_escape(label ? ACS_String_get(label) : "", esc, sizeof esc);
        printf("{\"id\":%d,\"x\":%d,\"y\":%d,\"w\":%d,\"h\":%d,\"label\":\"%s\"}%s",
               ACS_MeasurementShape_getId(sh), rc.x, rc.y, rc.width, rc.height, esc,
               (i+1<n_rects?",":""));
        ACS_String_free(label);
    }
    printf("],");

    // ellipses
    printf("\"ellipses\":[");
    for (size_t i=0; i<n_elps; ++i) {
        ACS_MeasurementEllipse* e = ACS_ListMeasurementEllipse_item(elps, i);
        const ACS_MeasurementShape* sh = ACS_MeasurementEllipse_asMeasurementShape(e);
        ACS_Point pos = ACS_MeasurementEllipse_getPosition(e);
        int rx = ACS_MeasurementEllipse_getRadiusX(e);
        int ry = ACS_MeasurementEllipse_getRadiusY(e);
        ACS_String* label = ACS_MeasurementShape_getLabel(sh);
        json_escape(label ? ACS_String_get(label) : "", esc, sizeof esc);
        printf("{\"id\":%d,\"x\":%d,\"y\":%d,\"rx\":%d,\"ry\":%d,\"label\":\"%s\"}%s",
               ACS_MeasurementShape_getId(sh), pos.x, pos.y, rx, ry, esc,
               (i+1<n_elps?",":""));
        ACS_String_free(label);
    }
    printf("],");

    // polylines (só quantidade, para não alongar muito – dá pra expandir pontos)
    printf("\"polylinesCount\":%zu", n_polys);
    printf("},");

    // caminho para a matriz
    json_escape(csv_path, esc, sizeof esc);
    printf("\"temperatureMatrixCsv\":\"%s\"", esc);

    printf("}\n");

    // limpeza
    if (spots) ACS_ListMeasurementSpot_free(spots);
    if (rects) ACS_ListMeasurementRectangle_free(rects);
    if (elps)  ACS_ListMeasurementEllipse_free(elps);
    if (polys) ACS_ListMeasurementPolyline_free(polys);
    if (ci) ACS_Image_CameraInformation_free(ci);
    if (img) ACS_ThermalImage_free(img);
    return 0;
}
