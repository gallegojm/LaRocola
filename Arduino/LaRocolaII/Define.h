
// Supprimer // dans la ligne suivante pour le déverminage
// #define DEBUG

// FILESSORT 0 : fichiers non ordonnés
// FILESSORT 1 : ordonnés alphabétiquement à la volée
// FILESSORT 2 : ordonnés dans fichiers files.lst (a faire...)
#define FILESSORT 0

// Dimension des chaines de caractères pour les noms de fichiers et répertoires
#define MAX_FILENAME  132
// Dimension des chaines de caractères pour sons 'bip' et 'erreur'
#define MAX_FILESOUND 20
// Dimension des chaines de caractères pour lire les fichiers
#define MAX_READBUF   256

#define MAX_UDP_TX_PACKET 256

// Définition des broches utilisées par les shields

#define STAT_LED_PIN   5             // pin for Status Led
#define ETH_CS_PIN    10             // pin for Chip Select of ethernet circuit 
#define MP3_CS_PIN     4             // pin for Chip Select of VS1053 mp3 chip of MP3 shield 
#define SD_CS_PIN      9             // pin for Chip Select of SD card of MP3 shield

// Codes des erreurs / status

#define CODE_SDRD  2  // Erreur d'initialisation du lecteur de carte / unable to initialize SD card reader
#define CODE_CONF  3  // Erreur de lecture du fichier de configuration / error reading config file
#define CODE_RMP3  4  // Erreur d'initialisation du module rMP3 / unable to initialize MP3 module
#define CODE_NET   5  // Erreur de connexion au réseau / unable to connect to the net
#define CODE_UDP   6  // Erreur d'initialisation du serveur UDP / unable to start UDP server
#define CODE_FILE  7  // unable to open/create file

