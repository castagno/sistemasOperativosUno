/*
 ============================================================================
 Name        : baash.c
 Author      : Castagno Gustavo Daniel (34582890)
 Version     : 0.5
 Copyright   : Open Source
 Description : Baash in C. Built-ins & execv Implemented. Pipes & Dup pending
 ============================================================================
 */

#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define bufferSize 999

/*
 * Recibe una inputString y reemplaza todas las ocurrencias de targetString
 * por replacementString devolviendo un puntero a la string resultante.
 * */
char *strReplace(char *inputString, char *targetString, char *replacementString){
	if(inputString == NULL || targetString == NULL){
		return NULL;
	}
	char *initialString = malloc((strlen(inputString) + 1) * sizeof(char));
	strcpy(initialString, inputString);

	char *substractString = malloc((strlen(targetString) + 1) * sizeof(char));
	strcpy(substractString, targetString);
	if(replacementString == NULL){
		replacementString = malloc(sizeof(char));
		strcpy(replacementString, "");
	}

	char *substituteString = malloc((strlen(replacementString) + 1) * sizeof(char));
	strcpy(substituteString, replacementString);

	char *outputString = malloc(sizeof(char) + sizeof(char));
	strcpy(outputString, "");

	char *findedString = initialString;
	char *previousString = initialString;
	do {
		findedString = strstr(previousString, substractString);
		char *startString;
		if(findedString != NULL){
			startString = findedString;
		} else {
			startString = initialString + strlen(initialString);
		}

		char *tempString = malloc((startString - previousString + strlen(substituteString) + 1) * sizeof(char));
		memset(tempString, '\0', (startString - previousString + strlen(substituteString) + 1) * sizeof(char));
		strncat(tempString, previousString, startString - previousString);
		if(findedString != NULL){
			strcat(tempString, substituteString);
		}

		char *outputStringCopy = malloc((strlen(outputString) + 1) * sizeof(char));
		strcpy(outputStringCopy, outputString);
		free(outputString);
		outputString = malloc((strlen(outputStringCopy) + strlen(tempString) + 1) * sizeof(char));
		strcpy(outputString, outputStringCopy);
		strcat(outputString, tempString);

		previousString = startString + strlen(substractString);
		free(tempString);
		free(outputStringCopy);
	} while (findedString != NULL);

	free(substractString);
	free(substituteString);
	return outputString;
}

/*
 * Obtiene el directorio Padre.
 * */
char *pathParent(char *currentPath){
	if(currentPath == NULL){
		return "/";
	}
	char *originPath = malloc((strlen(currentPath) + 1) * sizeof(char));
	strcpy(originPath, currentPath);

	char *outputPath = malloc((strlen(originPath) + 1) * sizeof(char));
	strcpy(outputPath, "");
	char *absolutePath = malloc((strlen(outputPath) + 1) * sizeof(char));
	strcpy(absolutePath, originPath);
	char *folder = strtok(absolutePath, "/");
	while(folder != NULL && strcmp(folder, strstr(originPath, folder)) != 0){
		strcat(outputPath, "/");
		strcat(outputPath, folder);
		folder = strtok(NULL, "/");
	}
	if(strcmp(outputPath, "") == 0){
		strcpy(outputPath, "/");
	}

	free(originPath);

	if(access(outputPath, F_OK) != 0){
		free(outputPath);
		return NULL;
	}

	return outputPath;
}

/*
 * Halla el directorio en el cual esta ubicado el archivo a ejecutar.
 * */
char *pathFinder(char *newPath, char *currentPath){
	if(newPath == NULL){
		return NULL;
	}
	char *finalPath = malloc((strlen(newPath) + 1) * sizeof(char));
	strcpy(finalPath, newPath);
	if(strstr(finalPath, "...") != NULL){
		printf("%s does not exist.\r\n", finalPath);
		free(finalPath);
		return NULL;
	}
	char *originPath = malloc((strlen(currentPath) + 1) * sizeof(char));
	char *outputPath = malloc((strlen(currentPath) + strlen(newPath) + strlen("/") + 1) * sizeof(char));
	if(currentPath == NULL){
		strcpy(originPath, "/");
	} else {
		strcpy(originPath, currentPath);
	}

	while(strstr(finalPath, "//") != NULL){
		finalPath = strReplace(finalPath, "//", "/");
	}
	while(strstr(finalPath, "././") != NULL){
		finalPath = strReplace(finalPath, "././", "./");
	}

	char *parentPath = originPath;
	while(strstr(finalPath, "..") != NULL){
		if(strcmp(finalPath, "..") == 0){
			parentPath = pathParent(parentPath);
			finalPath = strReplace(finalPath, "..", "./");
		} else if(strncmp(finalPath, "../..", strlen("../..")) == 0){
			parentPath = pathParent(parentPath);
			strcpy(finalPath, strstr(finalPath, "../..") + strlen("../"));
		} else if(strncmp(finalPath, "../", strlen("../")) == 0){
			parentPath = pathParent(parentPath);
			finalPath = strReplace(finalPath, "../", "./");
		} else if(strstr(finalPath, "/..") != finalPath){
			char *relativePath = malloc((strstr(finalPath, "/..") - finalPath + 1) * sizeof(char));
			strncpy(relativePath, finalPath, strstr(finalPath, "/..") - finalPath);
			if(strstr(relativePath, "./") != NULL){
				relativePath = strReplace(relativePath, "./", "");
			}
			char *combinedPath = malloc((strlen(parentPath) + strlen(relativePath) + strlen("/") + 1) * sizeof(char));
			strcpy(combinedPath, parentPath);

			if(strcmp(combinedPath, "/") == 0){
				strcat(combinedPath, relativePath);
			} else {
				strcat(combinedPath, "/");
				strcat(combinedPath, relativePath);
			}

			strcpy(parentPath, combinedPath);
			strcpy(finalPath, (strstr(finalPath, "/..") + sizeof(char)));

			free(combinedPath);
		} else {
			printf("%s does not exist.\r\n", finalPath);
		}
	}

	if(strstr(finalPath, "./") != NULL){
		finalPath = strReplace(finalPath, "./", "");
	}
	if(strncmp(finalPath, "/", strlen("/")) == 0){
		strcpy(outputPath, finalPath);
	} else if(strcmp(parentPath, "/") == 0){
		strcpy(outputPath, parentPath);
		strcat(outputPath, finalPath);
	} else if(strcmp(finalPath, "") != 0){
		strcpy(outputPath, parentPath);
		strcat(outputPath, "/");
		strcat(outputPath, finalPath);
	} else {
		strcpy(outputPath, parentPath);
	}
	if(outputPath[strlen(outputPath) - 1] == '/' && strcmp(outputPath, "/")){
		outputPath[strlen(outputPath) - 1] = '\0';
	}
	if(access(outputPath, F_OK) != 0){
		free(outputPath);
		return NULL;
	}

	return outputPath;
}

/*
 * Retorna un array de Strings separandolas donde aparece tokenString
 * */
char **strToArray(char *originalString, char *tokenString){
	if(originalString == NULL || tokenString == NULL){
		return NULL;
	}
	char *initialString = malloc((strlen(originalString) + 1) * sizeof(char));
	strcpy(initialString, originalString);
	char *divString = malloc((strlen(tokenString) + 1) * sizeof(char));
	strcpy(divString, tokenString);

	char **args = malloc(sizeof(char*));

	int i = 0;

	char *findedString = initialString;
	char *previousString = initialString;
	do {
		findedString = strstr(previousString, divString);
		char *startString;
		if(findedString != NULL){
			startString = findedString;
		} else {
			startString = initialString + strlen(initialString);
		}

		char *tempString = malloc((startString - previousString + 1) * sizeof(char));
		memset(tempString, '\0', (startString - previousString + 1) * sizeof(char));
		strncpy(tempString, previousString, startString - previousString);

//		printf("sizeof(args) : %d \n",(int) sizeof(args)/sizeof(args[0]));

		char **argsTemp = malloc((i + 1) * sizeof(char*));
		int j;

		for(j = 0; j < i; j++){
			argsTemp[j] = args[j];
		}
		argsTemp[i++] = tempString;

		args = argsTemp;

		previousString = startString + strlen(divString);
	} while (findedString != NULL);

	return args;
}

/*
 * Imprime el Prompt y se encarga de reconocer si los comandos son built-ins o
 * programas a ejecutar con parametros.
 * */
int main(int argc, char *argv[]) {
	char *envHome = getenv("HOME");
	char *pathString = getenv("PATH");
	char *currentPath = malloc((strlen(envHome) + 1) * sizeof(char));
	strcpy(currentPath, envHome);
	char *envUser = getenv("USER");
	chdir(currentPath);

	FILE *hostNameFile = fopen("/proc/sys/kernel/hostname", "r");
	if(hostNameFile == NULL){
		printf("No hostname file found!");
		free(currentPath);
		return 1;
	}
	char *buffer = malloc(bufferSize * sizeof(char));
	char *hostName = fgets(buffer, bufferSize, hostNameFile);
	hostName = strReplace(hostName, "\n", "");

	int exit = 0;
	while(exit == 0){
		printf("%s@", envUser);
		printf("%s ", hostName);
		if(strncmp(currentPath, envHome, strlen(envHome)) != 0){
			printf("%s", currentPath);
		} else if(strcmp(currentPath, envHome) == 0){
			printf("~");
		} else {
			printf("~%s", strReplace(currentPath, envHome, ""));
		}
		printf(" $ ");

		char *inputString = malloc(bufferSize * sizeof(char));
		fgets(inputString, bufferSize, stdin);
		inputString = strReplace(inputString, "\n", "");

		if(strcmp(inputString, "") == 0){
			continue;
		}
		if(strcmp(inputString, "exit") == 0){
			break;
		}
		if(strncmp(inputString, "cd ", strlen("cd ")) == 0){
			chdir(strReplace(inputString, "cd ", ""));
			getcwd(buffer, bufferSize);
			char *cwdString = malloc((strlen(buffer) + 1) * sizeof(char));
			strcpy(cwdString, buffer);

			currentPath = cwdString;

		} else {
			char **args;
			if(strstr(inputString, " ") != NULL && strstr(inputString, "\\ ") == NULL){
				char *arguments = malloc((strlen(inputString) - strlen(strstr(inputString, " "))) * sizeof(char));
				strcpy(arguments, strstr(inputString, " "));
				arguments = arguments + sizeof(char);
				inputString[strlen(inputString) - strlen(strstr(inputString, " "))] = '\0';

				char **argsTemp = strToArray(arguments, " ");

				args = malloc(((sizeof(argsTemp) / sizeof(argsTemp[0])) + 2) * sizeof(char*));
				args [0] = "";
				int n;
				for(n = 0; n < (sizeof(argsTemp) / sizeof(argsTemp[0])); n++){
					args[n + 1] = argsTemp[n];
				}
				args[(sizeof(argsTemp) / sizeof(argsTemp[0])) + 1] = 0;
			} else {
				args = malloc(2 * sizeof(char*));
				args[0] = "";
				args[1] = 0;
			}
			char *combinedPath = pathFinder(inputString, currentPath);

			if(access(combinedPath, X_OK) == 0){
				args[0] = combinedPath;
				execv(combinedPath, args);
			} else {

				char *findedString = pathString;
				char *previousString = pathString;
				do {
					findedString = strstr(previousString, ":");
					char *startString;
					if(findedString != NULL){
						startString = findedString;
					} else {
						startString = pathString + strlen(pathString);
					}

					char *tempString = malloc((startString - previousString + 1) * sizeof(char));
					memset(tempString, '\0', (startString - previousString + 1) * sizeof(char));
					strncpy(tempString, previousString, startString - previousString);
					char *combinedPath = pathFinder(inputString, tempString);

					if(access(combinedPath, X_OK) == 0){
						int child = fork();
						if(child == 0){
							args[0] = combinedPath;
							execv(combinedPath, args);
						} else {
							wait(0);
						}
						break;
					}

					previousString = startString + strlen(":");
				} while (findedString != NULL);
			}
		}
	}

	if(hostNameFile != NULL){
		fclose(hostNameFile);
	}
	free(buffer);
	return 0;
}
