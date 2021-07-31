#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>

#define LSB 0x1

typedef struct imageStruct{
    char *format;
    char *comments;
    int width;
    int height;
    int max;
    int *pixelMap;
}imageStruct;

/*
 * Allocate a pixel RGB matrix memory
 * @size image width * height
 */
int* makeMap(int size){
    int *pixelMap;
    pixelMap = (int*)malloc(3*size*sizeof(int));
    //printf("%s\n", "Pixel array memory allocated");
    return pixelMap;
}

imageStruct* getPPM(FILE *f){
    if(f==NULL){
        printf("%s\n","error getting PPM info");
    }
    // initialise max dimensions varaibles
    
    char *format = (char*)calloc(2, 1);
    int len = 100;
    char *comments = (char*)calloc(len, 1);
    int maxW = 0;
    int maxH = 0;
    int colorDepth = 0;

    // read and place all the PPM info in the struct
    if(!feof(f)){
        fscanf(f, "%s", format);
        //printf("%s\n", format);
        while(1){
            char *tmp = calloc(1000,1);
            if(fscanf(f, " %['#']c", tmp) == 1){
                fscanf(f, " %[^\n]s", tmp);            
                if((strlen(comments)+strlen(tmp))>len){
                    printf("%s\n", "comments buffer overflow... realocating memory");
                    char *tempComments = realloc(comments, 2*(strlen(comments)+strlen(tmp)));
                    if(tempComments == NULL){
                        printf("%s\n", "error reallocating the memory...");                        
                    }else{
                        len = 2*(strlen(comments)+strlen(tmp));
                        comments = tempComments;
                        printf("%s\n", "memory reallocated succesfully");
                    }
                }
                strcat(comments, "# ");
                strcat(comments, tmp);
                strcat(comments, "\n");
            }else{
                //printf("%s", comments);
                free(tmp);
                break;
            }            
        }

        fscanf(f, " %d", &maxW);
        fscanf(f, " %d", &maxH);
        fscanf(f," %d", &colorDepth);
     
        //printf("\n%s\n", "PPM INFO:");
        //printf("\nFORMAT:\t\t%s\n=======\n", format);
        //printf("WIDTH:\t\t%d\n=======\nHEIGHT:\t\t%d\n======\nPIXEL MAX:\t%d\n==========\n", maxW, maxH, colorDepth);
        //printf("COMMENTS:\n=========\n%s=========\n", comments);
  
    }

    // initialization of the pixelMap struct now that PPM info is known
    int *pix = makeMap(maxW*maxH);
    imageStruct *image = (imageStruct*)malloc(sizeof(imageStruct));
    image->format = format;
    image->width = maxW;
    image->height = maxH;
    image->max = colorDepth;
    image->comments = comments;
    image->pixelMap = pix;

    // loop for filling the pixelMap with values
    int j = 0;
    while(!feof(f)){
        //printf("%d\n", j);
        fscanf(f," %d", &pix[j]);
        
        /*
        printf("%d\t", pix[j]);

        if(j%(maxW*3) == (maxW*3)-1 && j > maxW){
            printf("%s", "|\n");
        }else if(j%3 == 2){
            printf("%s", "|\t");
        }
        */

        j++;
    }
    
    printf("%s\n","PPM Struct created succesfully");
    
    return image;
}

void showPPM(imageStruct *image){

        printf("\n=============\n| %s |\n=============\n", "PPM INFO:");
        printf("\nFORMAT:\t\t%s\n=======\n", image->format);
        printf("HEIGHT:\t\t%d\n=======\nWIDTH:\t\t%d\n======\nPIXEL MAX:\t%d\n==========\n", image->width, image->height, image->max);
        printf("COMMENTS:\n=========\n%s=========\n\nPIXELS:\n=======\n", image->comments);
        for(int j = 0; j < image->width*image->height*3; j++){
            printf("%d\t", image->pixelMap[j]);
            
            if(j%9 == 8){
                printf("%s", "|\n");
                printf("\nROW: %d COLUMN: %d\n|\t", j/(image->width*3)/3, j%(image->width*3)/3);
            }else if(j%3 == 2){
                printf("%s", "|\t");
            }
        }
        printf("\n%s\n", "END");
}

// 32 bit random number genereator
uint32_t rand32() {
    uint32_t result;
    for(int i = 0; i<32; i+=8){
        int n = rand();
        result <<= i;
        result |= n;
    }
    return result;
}

char* decode(imageStruct *image, uint32_t secret, int method){
    
    srand(secret);
    uint32_t pixelNum = image->width * image->height * 3;
    uint32_t stamp = 0;
    uint32_t index = 0;
    uint32_t counter = 0;
    uint32_t *indexArr = malloc(pixelNum/3*sizeof(uint32_t));

    uint8_t mask;
    mask = LSB << method;
    int shift = method;

    int len = 200;
    char *result = calloc(len, 1);


    int flag = 0;
    while(flag == 0){
        int ch = 0;
        for(int i = 0; i < 8; i++){

            // check if the counter is out of bounds
            if(counter > 0 && counter%(pixelNum-(pixelNum%3)) == 0){
                stamp = counter;
                memset((void*)indexArr, -1, sizeof(indexArr));
                if(method<4){
                    if(counter >= pixelNum*(8-method)-(pixelNum%3)){
                        printf("\nError, message length overflow... exiting\n");
                        exit(0);
                    }
                    //printf("\n==========================================================\n","");
                    //printf("\nIncresing mask level, old mask: %d \tnew mask: %d\n", mask, (mask << 1));
                    mask <<= 1;
                    shift++;
                    //printf("Shift: %d\n", shift);
                }else{
                    if(counter >= pixelNum*(method+1)-(pixelNum%3)){
                        printf("\nError, message length overflow... exiting\n");
                        exit(0);
                    }
                    //printf("\n==========================================================\n","");
                    //printf("\nDecresing mask level, old mask: %d \tnew mask: %d\n", mask, (mask >> 1));
                    mask >>= 1;
                    shift--;
                    //printf("Shift: %d\n", shift);
                }
            }

            //every RGB generate new index
            if(counter%3 == 0){

                index = rand32()%pixelNum;
                if(index>2){
                    index -= index%3; //round down to divisible by 3
                }else{
                    index = 0;
                }

                //chech if index has been used yet:
                for(int n = 0; n < (counter-stamp)/3; n++){   
                    if (indexArr[n] == index){
                        index = rand32()%pixelNum;
                        if(index>2){
                            index -= index%3;
                            //printf("index minus modulo: %d\t",index);
                        }else{
                            index = 0;
                        }
                        n = -1;
                    }
                }
                indexArr[(counter-stamp)/3] = index;
                //printf("index: \t%d\n", index);
            }

            int bit = 0;
            bit = (image->pixelMap[index+counter%3] & mask);
            
            //printf("bit value: %d\t", image->pixelMap[index+counter%3]);
            //printf("masked bit: %d\t", bit);
            if(shift-i >= 0){
                bit >>= (shift-i);
            }else{
                bit <<= (i-shift);
            }
            
            //printf("shifted bit: %d\t", bit);
            ch |= bit;
            //printf("char value: %d\t\n", ch);

            counter++;
        }


        if((strlen(result)+1)>=len){
            printf("%s\n", "result buffer overflow... realocating memory");
            char *temp = realloc(result, 2*(strlen(result)+1));
            if(temp == NULL){
                printf("%s\n", "error reallocating the memory...");                        
            }else{
                len = 2*(strlen(result)+1);
                result = temp;
                printf("%s\n", "memory reallocated succesfully");
            }
        }

        //printf("%c", ch);

        char tmp[1];
        sprintf(tmp, "%c", (char *)ch);
        strcat(result, (const char *)tmp);
        
        if(ch == '\0'){
            flag = 1;
        }
    }
    free(indexArr);
    return result;
}


/*
 * encode - creates new imageStruct with the message hidden in the image pixels
 * @parameters: imageStruct *image  - image struct to be modified;
 *              char *message       - message to icorporate in image pixels;
 *              uint32_t mSize      - message lenght;
 *              uint32_t secret     - key to encode/decode the message;
 *              char encoding       - encoding method to be utilised
 * @returns pointer to the modified imageStruct
 */
imageStruct* encode(imageStruct *image, char *message, uint32_t mSize, uint32_t secret, int method){

    srand(secret);
    uint32_t pixelNum;
    pixelNum = image->width * image->height * 3;
    uint32_t index = 0;
    uint32_t counter = 0;
    uint32_t counterStamp = 0;
    uint32_t indexArr[mSize*8+1];
    memset((void*)indexArr, -1, sizeof(indexArr));
    uint8_t mask;
    //printf("\nNumber of pixels: %d\n",pixelNum);
    mask = LSB << method;
    int shift = method;
    
    // image struct init
    imageStruct *cImage = (imageStruct*)malloc(sizeof(imageStruct));
    cImage->format = image->format;
    cImage->width = image->width;
    cImage->height = image->height;
    cImage->max = image->max;
    cImage->comments = image->comments;
    int *pix = makeMap(cImage->width*cImage->height);
    memcpy((void*) pix, (const void*) image->pixelMap, pixelNum*4);
    cImage->pixelMap = pix;

    // read the string char by char 
    for(int i=0; i<mSize+1; i++){
        
        // read chars bit by bit
        for(int j = 0; j < 8; j++){
            // check if the counter is out of bounds
            if(counter > 0 && counter%(pixelNum-(pixelNum%3)) == 0){
                counterStamp = counter;
                printf("\nCounter Stampped: %d", counterStamp);
                memset((void*)indexArr, -1, sizeof(indexArr));
                if(method<4){
                    if(counter >= pixelNum*(8-method)-(pixelNum%3)){
                        printf("\nError, image is to small to encode privided data\n");
                        printf("Maximum msg lenght for this image using encoding method: %d is: %d characters\nexiting...\n", method, (pixelNum*(8-method)-(pixelNum%3))/8);
                        exit(0);
                    }
                    //printf("\n==========================================================","");
                    printf("\nIncresing mask level, old mask: %d \tnew mask: %d\n", mask, (mask << 1));
                    shift++;
                    mask <<= 1;
                }else{
                    if(counter >= pixelNum*(method+1)-(pixelNum%3)){
                        printf("\nError, image is to small to encode privided data\n");
                        printf("Longest msg for this image using encoding method: %d is: %d characters\nexiting...\n", method, (pixelNum*(method+1)-(pixelNum%3))/8);
                        exit(0);
                    }                    
                    //printf("\n==========================================================","");
                    printf("\nDecresing mask level, old mask: %d \tnew mask: %d\n", mask, (mask >> 1));
                    shift--;
                    mask >>= 1;
                }
            }

            //printf("%d\n",counter);

            // every 3 bits ( RGB pixel ) generate new pixel index
            if(counter%3 == 0){    
                index = rand32()%pixelNum;
                if(index>2){
                    index -= index%3;
                }else{
                    index = 0;
                }
                //chech if index has been used yet:
                for(int n = 0; n <= (counter-counterStamp)/3; n++){
                    // if it has generate a new one and check again
                    if (indexArr[n] == index){
                        index = rand32()%pixelNum;
                        if(index>2){
                            index -= index%3;
                            //printf("reindex: %d\t",index);
                        }else{
                            index = 0;
                        }
                        n = -1;
                    }
                }
                // save valid index to used index array
                indexArr[(counter-counterStamp)/3] = index;
                //printf("\nindex: %d\t",index);
            }

            // bitwise check if the pixel's bit needs changing
            if(((message[i]>>j & LSB) << shift) != (pix[index+counter%3] & mask)){
            
                // if message's bit is set than OR pixel bit with mask to set it as well
                if( (message[i]>>j & LSB) ){
                    //printf("\tNEW PIXEL: %d", pix[index+counter%3] | mask);
                    pix[index+counter%3] = pix[index+counter%3] | mask;
                }
           
                // if message's bit is zero than AND it with ~mask (setting bit LOW and masking the other bits)
                if( !(message[i]>>j & LSB) ){
                    //printf("\tNEW PIXEL: %d", pix[index+counter%3] & ~mask);
                    pix[index+counter%3] = pix[index+counter%3] & (mask^255);
                }
            }
            counter++;
        }
        //printf("%c",message[i]);
    }
    return cImage;
}

void writeImStruct(imageStruct *im, FILE *f){
    fprintf(f, "%s\n", im->format);
    fprintf(f, "%s", im->comments);
    fprintf(f, "%d\n", im->width);
    fprintf(f, "%d\n", im->height);
    fprintf(f, "%d\n", im->max);
    for(int i = 0; i < im->width * im->height * 3; i++){
        fprintf(f, "%d", im->pixelMap[i]);
        if(i != 0 && i%3==2)
            fprintf(f, "%s", "\n");
        else
            fprintf(f, "%s", " ");
           
    }
    fprintf(f, "%s", "\0");
}

void encodeRoutine(int argc, char** argv){
    
    FILE *inputImage, *outputImage;
    
    inputImage = fopen(argv[2], "r");
    if(inputImage == NULL){
        printf("%s%s", argv[2], " file couldn't be opened\n");
        exit(0);
    }else{
        printf("\"%s\"%s", argv[2], " file opened\n");
    }
    
    outputImage = fopen(argv[3], "w");
    if(inputImage == NULL){
        printf("%s%s", argv[3], " file couldn't be created/opened\n");
        exit(0);
    }else{
        printf("\"%s\"%s", argv[3], " file opened\n");
    }
    

    int method = 0;

    sscanf(argv[4], " %d", &method);
    while(method>7 || method < 0){
        printf("Encoding method: %d is invalid please provide a value between 0 and 7:\n", method);
        char* tmp = malloc(5);
        fgets(tmp, 5, stdin);
        sscanf(tmp, " %d", &method);
        free(tmp);
    }
    
    int len = 15000;
    char* message = calloc(len,1);

    if(argc >5 && strcmp(argv[5], "-f") == 0){

        printf("Encoding method: %d \nProvide file path:\n", method);
        
        char* file = malloc(100);
        char* tmp = calloc(5000, 1);

        fgets(file,500,stdin);
        file[strcspn(file, "\r\n")] = 0;
        FILE *inputFile;
        inputFile = fopen(file, "r");
        if(inputFile == NULL){
            printf("%s%s", file, " file couldn't be opened\n");
            exit(0);
        }else{
            printf("\"%s\"%s", file, " file opened\n");
            free(file);
        }
        while(!feof(inputFile)){
            if(fscanf(inputFile, " %[^\n]s", tmp)>0)
                strcat(tmp, "\n");          
            if((strlen(message)+strlen(tmp))>len){
                printf("%s\n", "message buffer overflow... realocating memory");
                char *tempMsg = realloc(message, 2*(strlen(message)+strlen(tmp)));
                if(tempMsg == NULL){
                    printf("%s\n", "error reallocating the memory...");                        
                }else{
                    len = 2*(strlen(message)+strlen(tmp));
                    message = tempMsg;
                    printf("%s\n", "memory reallocated succesfully");
                }
            }
            strcat(message, tmp);
            //printf("%s\n", message);
        }
        //printf("%s\n", message);
        free(tmp);
    }else{
        printf("Encoding method: %d \nType in the secret message:\n", method);
        fgets(message,500,stdin);
    }

    if ((strlen(message) > 0) && (message[strlen (message) - 1] == '\n'))
        message[strlen(message) - 1] = '\0';
    else
        strcat(message, "\0");

    printf("MSG LEN: %d\n", strlen(message));
    //printf("FINAL MSG: %s\n", message);

    printf("%s\n", "Provide key for encoding");
    //getchar();
    char* skey = malloc(100);
    fgets(skey, 100, stdin);
    int key;
    sscanf(skey, " %d", &key);
    free(skey);

    imageStruct *image = getPPM(inputImage);
    if( strlen(message) < 5000)
        printf("Encoding message:\n\"%s\" \nSecret key: %d\n", message, key);
    else
        printf("Encoding message..\nSecret key: %d\n", key);
    
    imageStruct *cImage = encode(image, message, strlen(message), key, method);
    printf("Encoding succesfull... writing to file %s\n", argv[3]);
    //showPPM(cImage);
    writeImStruct(cImage, outputImage);

    free(image->pixelMap);
    free(image->format);
    free(image->comments);
    free(image);
    free(cImage->comments);
    free(cImage->format);
    free(cImage->pixelMap);
    free(cImage);
    free(message);
}

void decodeRoutine(char **argv){
    
    FILE *inputImage;

    inputImage = fopen(argv[2], "r");
    if(inputImage == NULL){
        printf("%s%s", argv[2], " file couldn't be opened\n");
        exit(0);
    }else{
        printf("\"%s\"%s", argv[2], " file opened\n");
    }

    int method;
    sscanf(argv[3], " %d", &method);

    while(method>7 || method < 0){
        printf("Encoding method: %d is invalid please provide a value between 0 and 7:\n", method);
        char* tmp = malloc(5);
        fgets(tmp, 5, stdin);
        sscanf(tmp, " %d", &method);
        free(tmp);
    }

    printf("Encoding method: %d\n", method);


    printf("%s\n", "Provide numeric key for decoding:");
    char* skey = malloc(100);
    fgets(skey, 100, stdin);
    int key;
    sscanf(skey, " %d", &key);
    free(skey);

    imageStruct *dImage = getPPM(inputImage);
    printf("Decoding using key: %d... \n", key);
    char* message = decode(dImage, key, method);
    printf("\n\nHidden message: \n%s\n\n", message);

    free(dImage->pixelMap);
    free(dImage->format);
    free(dImage->comments);
    free(dImage);
    free(message);
}

//#################################################################################
//#################################################################################

int main(int argc, char ** argv){
    
    if(strcmp(argv[1], "e")==0){
        encodeRoutine(argc, argv);
    }else if((strcmp(argv[1], "d"))==0){
        decodeRoutine(argv);
    }else if((strcmp(argv[1], "s"))==0){
        FILE *inputImage = fopen(argv[2], "r");
        if(inputImage == NULL){
            printf("%s%s", argv[2], " file couldn't be opened\n");
            exit(0);
        }else{
            printf("\"%s\"%s", argv[2], " file opened\n");
        }
        showPPM(getPPM(inputImage));
    }else{
        printf("%s\n", "#################################################################################");
        printf("\n%s\n", "Please provide valid arguments in the format:");
        printf("%s\n", "\t\".\\steg.exe cmd input-file-path output-file-path args\"");
        printf("\n%s\n", "cmd is required! valid cmd's: ");
        printf("%s\n", "\td - decode message from file (omit output-file-path when decoding)");
        printf("%s\n","\te - encode message in file (encoding method is required arg for encoding!");
        printf("%s\n", "\ts - show PPM struct\n\th - show this help message");
        printf("\n%s\n", "valid args: \n\t0-7 (0 is the LSB encoding and 7 is the MSB encoding)");
        printf("%s\n\n","\tfile-path (-f for encoding a file, nothing if message is to be typed in)");
        printf("%s\n\n", "#################################################################################");
    }
    printf("%s\n", "Complete!");
}