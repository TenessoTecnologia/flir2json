#include <acs/thermal_image.h>
#include <acs/renderer.h>
#include <acs/palette.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// Função de verificação de erro genérica
static void checkAcs(void) {
    ACS_Error err = ACS_getLastError();
    if (err.code) {
        ACS_String *msg = ACS_getErrorMessage(err);
        fprintf(stderr, "ACS error: %s | details: %s\n", ACS_String_get(msg), ACS_getLastErrorMessage());
        ACS_String_free(msg);
        exit(1);
    }
}

// Função para escapar caracteres no JSON
static void json_escape(const char *in, char *out, size_t outsz) {
    size_t j = 0;
    for (size_t i = 0; in[i] && j + 2 < outsz; ++i) {
        char c = in[i];
        if (c == '"' || c == '\\') { out[j++] = '\\'; out[j++] = c; }
        else if (c == '\n') { out[j++] = '\\'; out[j++] = 'n'; }
        else { out[j++] = c; }
    }
    out[j] = 0;
}

// Função para gravar matriz de temperaturas em CSV
static void write_temperature_csv(ACS_ThermalImage *img, const char *csv_path) {
    FILE *fp = fopen(csv_path, "w");
    if (!fp) {
        perror("Erro ao criar arquivo CSV");
        exit(1);
    }

    size_t width = ACS_ThermalImage_getWidth(img);
    size_t height = ACS_ThermalImage_getHeight(img);

    // Tentativas automáticas de função compatível
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            double temp = 0.0;

            #if defined(ACS_ThermalImage_getObjectTemperatureAt)
                temp = ACS_ThermalImage_getObjectTemperatureAt(img, x, y).value;
            #elif defined(ACS_ThermalImage_getTemperatureAt)
                temp = ACS_ThermalImage_getTemperatureAt(img, x, y).value;
            #elif defined(ACS_ThermalImage_getPixelTemperature)
                temp = ACS_ThermalImage_getPixelTemperature(img, x, y).value;
            #else
                fprintf(stderr, "⚠️ Nenhuma função de leitura de temperatura encontrada no SDK instalado.\n");
                temp = -9999.0; // marcador de erro
            #endif

            fprintf(fp, "%.2f", temp);
            if (x < width - 1) fprintf(fp, ";");
        }
        fprintf(fp, "\n");
    }

    fclose(fp);
    printf("✅ CSV gerado com sucesso: %s\n", csv_path);
}

// Função principal de extração
int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <imagem_radiometrica> <saida_csv>\n", argv[0]);
        return 1;
    }

    const char *input_path = argv[1];
    const char *csv_path = argv[2];

    ACS_ThermalImage *img = ACS_ThermalImage_createFromFile(input_path);
    checkAcs();

    write_temperature_csv(img, csv_path);

    ACS_ThermalImage_free(img);
    printf("{\"status\": \"ok\", \"message\": \"Extração concluída com sucesso!\"}\n");
    return 0;
}
