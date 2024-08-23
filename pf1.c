#include "pf1.h"

void borrarUltimaLinea() {

    FILE *archivo = fopen("sorted.txt", "r+");
    if (!archivo) {
        perror("Error al abrir el archivo");
        return;
    }

    fseek(archivo, 0, SEEK_END);
    unsigned int desplazamiento = ftell(archivo);

    char ch;
    while (desplazamiento > 0) {
        fseek(archivo, --desplazamiento, SEEK_SET);
        ch = fgetc(archivo);
        if (ch == '\n') {
            break;
        }
    }

    if (desplazamiento > 0) {
        ftruncate(fileno(archivo), desplazamiento);
    } else {
        ftruncate(fileno(archivo), 0);
    }

    fclose(archivo);
}

int esLineaVacia(const char *str) {
    while (*str) {
        if (!isspace((unsigned char)*str)) {
            return 0;
        }
        str++;
    }
    return 1;
}

int compararStrings(const char* s1, const char* s2) {
    if (!s1 || !s2) return -1;

    int cmp = -strcasecmp(s1, s2);
    if (cmp != 0) {
        return cmp;
    } else {
        return -strcmp(s1, s2);
    }
}

int compararStringsParaqsort(const void *a, const void *b) {
    const char *str_a = *(const char **)a;
    const char *str_b = *(const char **)b;
    
    int cmp = -strcasecmp(str_a, str_b);
    if (cmp != 0) {
        return cmp;
    } else {
        return -strcmp(str_a, str_b);
    }
}

void *funcionTrabajadora(void *arg) {
    thread_data_t *data = (thread_data_t *)arg;
    if (!data || !data->nombreArchivo || !data->stats) {
        fprintf(stderr, "Error\n");
        return NULL;
    }

    FILE *archivo = fopen(data->nombreArchivo, "r");
    if (!archivo) {
        perror("fopen");
        return NULL;
    }

    char *linea = NULL;
    size_t longitud = 0;
    ssize_t leer;
    char **lineas = NULL;
    unsigned int contador = 0;
    size_t longitudMasLarga = 0, longitudMasCorta = SIZE_MAX;

    while ((leer = getline(&linea, &longitud, archivo)) != -1) {
        if (linea[leer - 1] == '\n') {
            linea[leer - 1] = '\0';
        }

        if (esLineaVacia(linea)) continue;

        if (strlen(linea) == 0) continue;
        char *lineaTemp = strdup(linea);
        if (!lineaTemp) {
            perror("strdup");
            goto cleanup;
        }
        char **lineasTemp = realloc(lineas, (contador + 1) * sizeof(char *));
        if (!lineasTemp) {
            perror("realloc");
            free(lineaTemp);
            goto cleanup;
        }
        lineas = lineasTemp;
        lineas[contador++] = lineaTemp;

        size_t longitudDeLinea = strlen(linea);
        if (longitudDeLinea > longitudMasLarga || (longitudDeLinea == longitudMasLarga && compararStrings(linea, data->stats->lineaMasLarga) > 0)) {
            longitudMasLarga = longitudDeLinea;
            free(data->stats->lineaMasLarga);
            data->stats->lineaMasLarga = strdup(linea);
        }
        if (longitudDeLinea < longitudMasCorta || (longitudDeLinea == longitudMasCorta && compararStrings(linea, data->stats->lineaMasCorta) > 0)) {
            longitudMasCorta = longitudDeLinea;
            free(data->stats->lineaMasCorta);
            data->stats->lineaMasCorta = strdup(linea);
        }
    }

    qsort(lineas, contador, sizeof(char *), compararStringsParaqsort);

    char nombreArchivoOrdenado[500];
    if (snprintf(nombreArchivoOrdenado, sizeof(nombreArchivoOrdenado), "%s.sorted", data->nombreArchivo) >= sizeof(nombreArchivoOrdenado)) {
        fprintf(stderr, "Nombre de archivo muy largo\n");
        goto cleanup;
    }

    FILE *archivoOrdenado = fopen(nombreArchivoOrdenado, "w");
    if (!archivoOrdenado) {
        perror("fopen");
        goto cleanup;
    }

    for (unsigned int i = 0; i < contador; i++) {
        if (i == contador - 1) {
            if (longitud > 0) {
                fprintf(archivoOrdenado, "%s", lineas[i]);
                free(lineas[i]);
            }
            break;
        }
        fprintf(archivoOrdenado, "%s\n", lineas[i]);
        free(lineas[i]);
    }
    fclose(archivoOrdenado);

    data->stats->numLineasOrdenadas = contador;

    printf("This worker thread writes %u lineas to \"%s\"\n", contador, nombreArchivoOrdenado);

cleanup:
    if (archivo) fclose(archivo);
    if (linea) free(linea);
    if (lineas) free(lineas);
    return NULL;
}

unsigned int contarLineas(FILE* archivo)
{
    ssize_t leer;
    unsigned int contador = 0;
    size_t longitud = 0;
    char* linea = NULL;

    rewind(archivo);
    
    while((leer = getline(&linea, &longitud, archivo)) != -1) {
        contador++;
    }

    free(linea);
    rewind(archivo);
    return contador;
}

FILE* mergeOrdenados(FILE* archivo1, FILE* archivo2) {
    unsigned int lineasarchivo1 = contarLineas(archivo1), lineasarchivo2 = contarLineas(archivo2);
    FILE* temporal = tmpfile();
    ssize_t leer1 = 0, leer2 = 0;
    size_t longitud1 = 0, longitud2 = 0;
    char *c1 = NULL, *c2 = NULL;
    char *ultimaEscrita = NULL;
    unsigned int contadorLineas = 0;

    rewind(archivo1);
    rewind(archivo2);

    leer1 = getline(&c1, &longitud1, archivo1);
    leer2 = getline(&c2, &longitud2, archivo2);

    while (leer1 != -1 || leer2 != -1) {

        if (leer1 != -1 && c1[strlen(c1) - 1] != '\n') {
            strcat(c1, "\n");
        }
        if (leer2 != -1 && c2[strlen(c2) - 1] != '\n') {
            strcat(c2, "\n");
        }


        if (leer1 != -1 && (leer2 == -1 || compararStrings(c1, c2) <= 0)) {
            if (ultimaEscrita == NULL || strcmp(c1, ultimaEscrita) != 0) {
                fprintf(temporal, "%s", c1);
                free(ultimaEscrita);
                ultimaEscrita = strdup(c1);
                contadorLineas++;
            }
            leer1 = getline(&c1, &longitud1, archivo1);
        } else if (leer2 != -1) {
            if (ultimaEscrita == NULL || strcmp(c2, ultimaEscrita) != 0) {
                fprintf(temporal, "%s", c2);
                free(ultimaEscrita);
                ultimaEscrita = strdup(c2);
                contadorLineas++;
            }
            leer2 = getline(&c2, &longitud2, archivo2);
        }
    }

    rewind(archivo1);
    rewind(archivo2);

    fprintf(stdout, "Merged %u lines and %u lines into %u lines.\n", lineasarchivo1, lineasarchivo2, contadorLineas);

    rewind(temporal);

    free(c1);
    free(c2);
    free(ultimaEscrita);

    return temporal;
}

void *mergeHilo(void *arg) {
    merge_data_t *data = (merge_data_t *)arg;
    FILE *mergedArchivo = mergeOrdenados(data->archivo1, data->archivo2);
    fclose(data->archivo1);
    fclose(data->archivo2);
    pthread_exit((void *)mergedArchivo);
}

void mergeArbol(FILE **archivosOrdenados, int num, unsigned int *contadoresDeLineas) {
    while (num > 1) {
        int mergedContador = (num + 1) / 2;
        pthread_t *threads = (pthread_t *)malloc(mergedContador * sizeof(pthread_t));
        merge_data_t *mergeDatos = (merge_data_t *)malloc(mergedContador * sizeof(merge_data_t));
        FILE **nuevosArchivos = (FILE **)malloc(mergedContador * sizeof(FILE *));
        unsigned int *nuevoContadorDeLinea = (unsigned int *)malloc(mergedContador * sizeof(int));

        for (int i = 0; i < mergedContador; ++i) {
            if (2 * i + 1 < num) {
                mergeDatos[i].archivo1 = archivosOrdenados[2 * i];
                mergeDatos[i].archivo2 = archivosOrdenados[2 * i + 1];
                pthread_create(&threads[i], NULL, mergeHilo, (void *)&mergeDatos[i]);
            } else {
                nuevosArchivos[i] = archivosOrdenados[2 * i];
                nuevoContadorDeLinea[i] = contadoresDeLineas[2 * i];
            }
        }

        for (int i = 0; i < mergedContador; ++i) {
            if (2 * i + 1 < num) {
                pthread_join(threads[i], (void **)&nuevosArchivos[i]);
                nuevoContadorDeLinea[i] = contadoresDeLineas[2 * i] + contadoresDeLineas[2 * i + 1];
            }
        }

        free(archivosOrdenados);
        free(contadoresDeLineas);
        archivosOrdenados = nuevosArchivos;
        contadoresDeLineas = nuevoContadorDeLinea;
        num = mergedContador;

        free(threads);
        free(mergeDatos);
    }

    FILE *final_file = fopen("sorted.txt", "w");
    if (!final_file) {
        perror("fopen");
        return;
    }

    char *linea = NULL;
    size_t longitud = 0;
    while (getline(&linea, &longitud, archivosOrdenados[0]) != -1) {
        fprintf(final_file, "%s", linea);
    }
    fclose(final_file);
    fclose(archivosOrdenados[0]);
    free(archivosOrdenados);
    free(contadoresDeLineas);
    free(linea);
}

int main(int argc, char *argv[]) {
    if (argc < 3)
        error(0);

    pthread_t *threads = malloc((argc - 1) * sizeof(pthread_t));
    thread_data_t *dataThread = malloc((argc - 1) * sizeof(thread_data_t));
    stats_t *stats = malloc((argc - 1) * sizeof(stats_t));

    for (int i = 1; i < argc; ++i) {
        dataThread[i - 1].nombreArchivo = argv[i];
        dataThread[i - 1].stats = &stats[i - 1];

        stats[i - 1].lineaMasLarga = NULL;
        stats[i - 1].lineaMasCorta = NULL;

        pthread_create(&threads[i - 1], NULL, funcionTrabajadora, &dataThread[i - 1]);
    }

    for (unsigned int i = 1; i < argc; ++i) {
        pthread_join(threads[i - 1], NULL);
    }

    FILE **archivosOrdenados = (FILE **)malloc((argc - 1) * sizeof(FILE *));
    unsigned int *contadoresDeLineas = (unsigned int *)malloc((argc - 1) * sizeof(unsigned int));
    for (int i = 1; i < argc; ++i) {
        char nombreArchivoOrdenado[500];
        snprintf(nombreArchivoOrdenado, sizeof(nombreArchivoOrdenado), "%s.sorted", argv[i]);
        archivosOrdenados[i - 1] = fopen(nombreArchivoOrdenado, "r");
        contadoresDeLineas[i - 1] = stats[i - 1].numLineasOrdenadas;
    }

    mergeArbol(archivosOrdenados, argc - 1, contadoresDeLineas);

    char *lineaMasLarga = NULL;
    char *lineaMasCorta = NULL;
    size_t longitudMasLarga = 0, longitudMasCorta = SIZE_MAX;
    unsigned int lineasTotales = 0;

    for (int i = 0; i < argc - 1; ++i) {
        lineasTotales += stats[i].numLineasOrdenadas;
        if (stats[i].lineaMasLarga) {
            size_t longitud = strlen(stats[i].lineaMasLarga);
            if (longitud > longitudMasLarga || (longitud == longitudMasLarga && compararStrings(stats[i].lineaMasLarga, lineaMasLarga) > 0)) {
                longitudMasLarga = longitud;
                lineaMasLarga = stats[i].lineaMasLarga;
            }
        }
        if (stats[i].lineaMasCorta) {
            size_t longitud = strlen(stats[i].lineaMasCorta);
            if (longitud < longitudMasCorta || (longitud == longitudMasCorta && compararStrings(stats[i].lineaMasCorta, lineaMasCorta) > 0)) {
                longitudMasCorta = longitud;
                lineaMasCorta = stats[i].lineaMasCorta;
            }
        }
    }

    borrarUltimaLinea();

    printf("A total of %u strings were passed as input,\nlongest string sorted: %s\nshortest string sorted: %s\n",
        lineasTotales, lineaMasLarga, lineaMasCorta);


    free(threads);
    free(dataThread);
    free(stats);

    return 0;
}
