//////////////////////////////////////////////////////////////////////////////////////////////////////////
//   pcfg_manager - creates password guesses using a probablistic context free grammar
//
//   Copyright (C) Matt Weir <weir@cs.fsu.edu>
//
//   pcfg_manager.h 
//
//

#include "pcfg_manager.h"
#include <ctime>
unsigned long long crash_recovery_pos=0;  //used to restart an aborted session at the last compleated popped pre-terminal value
std::string structures_file;
long long cur_guesses = 0;
ofstream fout;
//----Eventually want to build in a way to save progress or check status----------//
void sig_alrm(int signo)
{
  printf("exiting\n");
  exit(0);
}



class queueOrder {
  public:
    queueOrder() {
    }
  bool operator() (const pqReplacementType& lhs, const pqReplacementType& rhs) const{
    return (lhs.probability<rhs.probability);
  }
};

typedef priority_queue <pqReplacementType,vector <pqReplacementType>,queueOrder> pqueueType;
bool processBasicStruct(pqueueType *pqueue,ntContainerType **dicWords,  ntContainerType **numWords, ntContainerType **specialWords);
bool generateGuesses(pqueueType *pqueue,bool isLimit,long long maxGuesses);
int createTerminal(pqReplacementType *curQueueItem,bool isLimit,long long  *maxGuesses, int workingSection, string *curOutput);
bool pushNewValues(pqueueType *pqueue,pqReplacementType *curQueueItem);
void help();  //prints out the usage info

//Process the input Dictionaries
bool processDic(string *inputDicFileName,bool *inputDicExists, double *inputDicProb, ntContainerType **dicWords,bool removeUpper, bool removeSpecial, bool removeDigits);
bool processProbFromFile(ntContainerType **numWords,const char *fileType);  //processes the number probabilities
short findSize(string input);  //used to find the length of a possible non-ascii string, used because MACOSX had problems with wstring

clock_t start_time, end_time;


int main(int argc, char *argv[]) {

  string inputDicFileName[MAXINPUTDIC];
  bool inputDicExists[MAXINPUTDIC];
  double inputDicProb[MAXINPUTDIC];

  ntContainerType *dicWords[MAXWORDSIZE];
  ntContainerType *numWords[MAXWORDSIZE];
  ntContainerType *specialWords[MAXWORDSIZE];
  long long maxGuesses=0;
  bool isLimit=false;
  pqueueType pqueue;
  
  bool removeUpper=false;   //if we should not include dictionary words that contain uppercase letters
  bool removeSpecial=false; //if we should not include dictionary words that contain special characters
  bool removeDigits=false;  //if we should not include dictionary words that contain digits
  bool crashRecovery=false;
  std::string model_dir = "";
  std::string grammar_dir = "grammar";
  std::string digits_dir = "digits";
  std::string symbol_dir = "special";
  std::string guesses_file = "";
//---------Parse the command line------------------------//

  //--Initilize the command line variables -- //
  for (int i=0;i<MAXINPUTDIC;i++) {
    inputDicExists[i]=false;
    inputDicProb[i]=1;
  }
 
  if (argc == 1) {
    help();
    return 0;
  }
  for (int i=1;i<argc;i++) {
    string commandLineInput = argv[i];
    string tempString = commandLineInput;
    int currentDic;
    if (commandLineInput.find("-dname")==0) { //reading in an input dictionary 
      tempString=commandLineInput.substr(6,commandLineInput.size());
      if (isdigit(tempString[0])) {
        currentDic=atoi(tempString.c_str());
        if (currentDic >=MAXINPUTDIC) {
          cout << "\nSorry, but the category of input dictionaries must fall between 0 and " << MAXINPUTDIC -1 << "\n";
          help();
          return 0;
        }
      }
      else {
        help();
        return 0;
      }
      i++;
      if (i<argc) {
        commandLineInput=argv[i];
        inputDicExists[currentDic]=true;
        inputDicFileName[currentDic]=commandLineInput;
      }
      else {
        cout << "\nSorry, but you need to include the filename after the -dname option\n";
        help();
        return 0;
      }
    }
    else if (commandLineInput.find("-dprob")==0) { //reading in a dictionary's probability
      tempString=commandLineInput.substr(6,commandLineInput.size());
      if (isdigit(tempString[0])) {
        currentDic=atoi(tempString.c_str());
        if (currentDic >=MAXINPUTDIC) {
          cout << "\nSorry, but the category of input dictionaries must fall between 0 and " << MAXINPUTDIC -1 << "\n";
          help();
          return 0;
        }
      }
      else {
        help();
        return 0;
      }
      i++;
      if (i<argc) {
        inputDicProb[currentDic]=atof(argv[i]);
        if ((inputDicProb[currentDic]>1.0)||(inputDicProb[currentDic]<=0)) {
          cout << "\nSorry, but the input dictionary probability must fall between 1.0 and 0, and not equal 0.\n";
          help();
          return 0;
        }
      }
      else {
        cout << "\nSorry, but you need to include the filename after the -dname option\n";
        help();
        return 0;
      }
    }
    else if (commandLineInput.find("-removeUpper")!=string::npos) {
      removeUpper=true;
    }
    else if (commandLineInput.find("-removeSpecial")!=string::npos) {
      removeSpecial=true;
    }
    else if (commandLineInput.find("-removeDigits")!=string::npos) {
      removeDigits=true;
    } else if (commandLineInput.find("--model") == 0) {
        i += 1;
        if (i < argc) {
            model_dir = argv[i];
        } else {
            help();
            return 1;
        }
    } else if (commandLineInput.find("--grammar") == 0) {
        i += 1;
        if (i < argc) {
            grammar_dir = argv[i];
        }
    } else if (commandLineInput.find("--digits") == 0) {
        i += 1;
        if (i < argc) {
            digits_dir = argv[i];
        }
    } else if (commandLineInput.find("--symbol") == 0) {
        i += 1;
        if (i < argc) {
            symbol_dir = argv[i];
        }
    } else if (commandLineInput.find("--guess-number") == 0) {
        i += 1;
        if (i < argc) {
            maxGuesses = atol(argv[i]);
        } else {
            help();
            return 1;
        }
    } else if (commandLineInput.find("--guesses-file") == 0) {
        i += 1;
        if (i < argc) {
            guesses_file = argv[i];
        } else {
            help();
            return 1;
        }
    }
    else {
      cout << "\nSorry, unknown command line option entered\n";
      help();
      return 0;
    }
  }
  if (model_dir.size() == 0 || maxGuesses == 0 || guesses_file.size() == 0) {
    help();
    return 1;
  }
  fout.open(guesses_file.c_str());
  if (!fout.is_open()) {
    std::cerr << "cannot open " << guesses_file << std::endl;
    return 1;
  }
  #ifdef _WIN32
  structures_file = model_dir + "\\" + grammar_dir + "\\" + "structures.txt";
  #else
  structures_file = model_dir + "/" + grammar_dir + "/" + "structures.txt";
  #endif

  //---------Process all the Dictioanry Words------------------//
  if (processDic(inputDicFileName, inputDicExists, inputDicProb,dicWords,removeUpper,removeSpecial,removeDigits)==false) {
    cout << "\nThere was a problem opening the input dictionaries\n";
    help();
    return 0;
  }

  #ifdef _WIN32
  digits_dir = model_dir + "\\" + digits_dir + "\\";
  if (processProbFromFile(numWords, digits_dir.c_str())==false) {
  #else
  digits_dir = model_dir + "/" + digits_dir + "/";
  if (processProbFromFile(numWords,digits_dir.c_str())==false) {
  #endif
    cout << "\nCould not open the number probability files\n";
    return 0;
  }
  #ifdef _WIN32
  symbol_dir = model_dir + "\\" + symbol_dir + "\\";
  if (processProbFromFile(specialWords, symbol_dir)==false) {
  #else
  symbol_dir = model_dir + "/" + symbol_dir + "/";
  if (processProbFromFile(specialWords,symbol_dir.c_str())==false) {
  #endif
    cout << "\nCould not open the special character probability files\n";
    return 0;
  }
  if (processBasicStruct(&pqueue, dicWords, numWords, specialWords)==false) {
    cout << "\nError, could not open structure file from the training set\n";
    return 0;
  }
  start_time = clock();
  if (generateGuesses(&pqueue,isLimit,maxGuesses)==false) {
    cout << "\nError generating guesses\n";
    return 0;
  }
  
  fout.flush();
  fout.close();

//  while (!pqueue.empty()) {
//    pqReplacementStruct temp;
//    temp  = pqueue.top();
//    pqueue.pop();
//    cout << setprecision(15) << temp.probability << endl;
//  }
  return 0;
}     


void help() {
  cout << endl << endl << endl;
  cout << "PCFG MANAGER - A password guess generator based on probablistic context free grammars\n";
  cout << "Created by Matt Weir, weir@cs.fsu.edu\n";
  cout << "Special thanks to Florida State University and the National Institute of Justice for supporting this work\n";
  cout << "----------------------------------------------------------------------------------------------------------\n";
  cout << "Usage Info:\n";
  cout << "./pcfg_manager <options>\n";
  cout << "\tOptions:\n";
  std::cout << "\t--model <model directory>\t<REQUIRED>: The trained model path\n";
  std::cout << "\t--guesses-file <guesses file>\t<REQUIRED>: guesses generated will be put here\n";
  std::cout << "\t--guess-number <guess number>\t<REQUIRED>: the guess number\n";
  std::cout << "\t--grammar <grammar folder>\t<OPTIONAL>: grammar directory name under model path\n";
  std::cout << "\t--digits <digits folder>\t<OPTIONAL>: digits directory name under model path\n";
  std::cout << "\t--symbol <symbol folder>\t<OPTIONAL>: symbol directory name under model path\n";
  cout << "\t-dname[0-" << MAXINPUTDIC-1 << "] <dictionary name>\t<REQUIRED>: The input dictionary name\n";
  cout << "\t\tExample: -dname0 common_words.txt\n";
  cout << "\t-dprob[0-" << MAXINPUTDIC-1 << "] <dictionary probability>\t<OPTIONAL>: The input dictionary's probability, if not specified set to 1.0\n";
  cout << "\t\tExample: -dprob0 0.75\n";
  cout << "\t-removeUpper\t\t<OPTIONAL>: don't include dictionary words that contain uppercase letters\n";
  cout << "\t-removeSpecial\t\t<OPTIONAL>: don't include dictionary words that contain special characters\n";
  cout << "\t-removeDigits\t\t<OPTIONAL>: don't include dictionary words that contain digits\n";
  cout << endl << endl;
  return;
}

bool compareDicWords(mainDicHolderType first, mainDicHolderType second) {
  int compareValue = first.word.compare(second.word);
  if (compareValue<0) {
    return true;
  }
  else if (compareValue>0) {
    return false;
  }
  else if (first.probability>second.probability) {
    return true;
  }
  return false;
}

bool duplicateDicWords(mainDicHolderType first, mainDicHolderType second) {
  if (first.word.compare(second.word)==0) {
    return true;
  }
  return false;
}

bool processDic(string *inputDicFileName, bool *inputDicExists, double *inputDicProb,ntContainerType **dicWords, bool removeUpper, bool removeSpecial, bool removeDigits) {
  ifstream inputFile;
  bool atLeastOneDic=false;  //just checks to make sure at least one input dictionary was specified
  mainDicHolderType tempWord;  
  list <mainDicHolderType> allTheWords;
  size_t curPos;
  int numWords[MAXINPUTDIC][MAXWORDSIZE];
  double wordProb[MAXINPUTDIC][MAXWORDSIZE];
  ntContainerType *tempContainer;  
  ntContainerType *curContainer; 
  bool goodWord=true;

  //-initilize the variables--//
  for (int i=0;i<MAXINPUTDIC;i++) {
    for (int j=0; j<MAXWORDSIZE;j++) {
      numWords[i][j]=0;
    }
  }

  for (int i=0; i<MAXINPUTDIC;i++) {  //for every input dictionary
    if (inputDicExists[i]) {
      inputFile.open(inputDicFileName[i].c_str());
      if (!inputFile.is_open()) {
        cout << "Could not open file " << inputDicFileName[i] << endl;
        return false;
      }
      tempWord.category=i;
      while (!inputFile.eof()) {
        std::getline(inputFile,tempWord.word);
        curPos=tempWord.word.find("\r");  //remove carrige returns
        if (curPos!=string::npos) {
          tempWord.word.resize(curPos);
        }
        tempWord.word_size=findSize(tempWord.word);
        if ((tempWord.word_size>0)&&(tempWord.word_size<MAXWORDSIZE)) {
          if ((removeUpper)||(removeSpecial)||(removeDigits)) {
            goodWord=true;
            if (removeUpper) {   //check to see if there are any uppercase letters
              for (int j=0;j<tempWord.word.size();j++) {
                if (((int)tempWord.word[j]>64)&&((int)tempWord.word[j]<91)) {
                  goodWord=false;
                  break;
                }
              }
            }
            if (removeSpecial) { //check to see if there are any special characters in the word
              for (int j=0;j<tempWord.word.size();j++) {
                if (((int)tempWord.word[j]<48)||(((int)tempWord.word[j]>57)&&((int)tempWord.word[j]<65))||(((int)tempWord.word[j]>90)&&((int)tempWord.word[j]<97))||(((int)tempWord.word[j]>122)&&((int)tempWord.word[j]<127))) {
                  goodWord=false;
                  break;
                }
              }
            }
            if (removeDigits) {
              for (int j=0;j<tempWord.word.size();j++) {
                if (((int)tempWord.word[j]>47)&&((int)tempWord.word[j]<58)) {
                  goodWord=false;
                  break;
                }
              }
            }
            if (goodWord) {
              allTheWords.push_front(tempWord);
              numWords[i][tempWord.word_size]++;
            }
            else {
//              cout <<"bad word = " << tempWord.word << endl;
            }
          } 
          else {
            allTheWords.push_front(tempWord);
            numWords[i][tempWord.word_size]++;
          }
        }
      }
      atLeastOneDic=true;
      inputFile.close();
    }
  }
  if (!atLeastOneDic) {
    return false;
  }
  //--Calculate probabilities --//
  for (int i=0; i<MAXINPUTDIC;i++) {
    for (int j=0; j<MAXWORDSIZE;j++) {
      if (numWords[i][j]==0) {
        wordProb[i][j]=0;
      }
      else {
        wordProb[i][j]= inputDicProb[i] * (1.0/numWords[i][j]);
      }
    }
  }
  list <mainDicHolderType>::iterator it;
  for (it=allTheWords.begin();it!=allTheWords.end();++it) {
    (*it).probability = wordProb[(*it).category][(*it).word_size]; 
  }
  
  allTheWords.sort(compareDicWords);
  allTheWords.unique(duplicateDicWords);

  //------Now divide the words into their own ntStructures-------//
  for (int i=0;i<MAXWORDSIZE;i++) {
    dicWords[i]=NULL;
    for (int j=0;j<MAXINPUTDIC;j++) {
      if (wordProb[j][i]!=0) {
        tempContainer = new ntContainerType;
        tempContainer->next=NULL;
        tempContainer->probability=wordProb[j][i];
        tempContainer->replaceRule=false;
        if (dicWords[i]==NULL) {
          dicWords[i]= tempContainer;
        }
        else if (dicWords[i]->probability<tempContainer->probability) {
          tempContainer->next = dicWords[i];
          dicWords[i]=tempContainer;
        }
        else {
          curContainer=dicWords[i];
          while ((curContainer->next!=NULL)&&(curContainer->next->probability>tempContainer->probability)) {
            curContainer=curContainer->next;
          }
          tempContainer->next=curContainer->next;
          curContainer->next=tempContainer;
        }
      }
    }
  }
  for (it=allTheWords.begin();it!=allTheWords.end();++it) {
    tempContainer=dicWords[(*it).word_size];
    while ((tempContainer!=NULL)&&(tempContainer->probability!=(*it).probability)) {
      tempContainer=tempContainer->next;
    }
    if (tempContainer==NULL) {
      cout << "Error with processing input dictionary\n";  
      cout << "Word " <<(*it).word << " prob " <<(*it).probability << endl;
      return false;
    }
    tempContainer->word.push_back((*it).word);
  }


  return true;
}

short findSize(string input) { //used to find the size of a string that may contain wchar info, not using wstrings since MACOSX is being stupid
                               //aka when I built it on Ubuntu it worked with wstring, but mac still reads them in as 8 bit chars
  short size=0;
  for (int i=input.size()-1;i>=0;i--) {
    if ((unsigned int)input[i]>127) { //it is a non-ascii char
      i--;
    }
    size++;
  }
  return size;
}


bool processProbFromFile(ntContainerType **mainContainer, const char *type) {  //processes the number probabilities
  bool atLeastOneValue=false;
  ifstream inputFile;
  list <string>::iterator it;  
  char fileName[256];
  ntContainerType * curContainer;
  string inputLine;
  size_t marker;
  double prob;

  for (int i=0; i<MAXWORDSIZE;i++) {
    sprintf(fileName,"%s%i.txt",type,i);
    inputFile.open(fileName);
    if (inputFile.is_open()) { //a file exists for that string length
      curContainer=new ntContainerType;
      curContainer->next=NULL;
      curContainer->replaceRule=false;
      mainContainer[i]=curContainer;
      mainContainer[i]->probability=0;
      while (!inputFile.eof()) {
        getline(inputFile,inputLine);
        marker=inputLine.find("\t");
        if (marker!=string::npos) {
          prob=atof(inputLine.substr(marker+1,inputLine.size()).c_str());
          if ((curContainer->probability==0)||(curContainer->probability==prob)) {
            curContainer->probability=prob;
            curContainer->word.push_back(inputLine.substr(0,marker));
          }
          else {
            curContainer->next=new ntContainerType;
            curContainer=curContainer->next;
            curContainer->next=NULL;
            curContainer->replaceRule=false;
            curContainer->probability=prob;
            curContainer->word.push_back(inputLine.substr(0,marker));
          }
        }

      }
      atLeastOneValue=true; 
      inputFile.close();         
    }
    else {
      mainContainer[i]=NULL; 
    }
  }
  if (!atLeastOneValue) {
    cout << "Error trying to open the probability values from the training set\n";
    return false;
  }
  return true;
}


bool processBasicStruct(pqueueType *pqueue,ntContainerType **dicWords, ntContainerType **numWords, ntContainerType **specialWords) {
  ifstream inputFile;
  string inputLine;
  size_t marker;
  double prob;
  int valueLen;
  pqReplacementType inputValue;
  char pastCase='!';
  int curSize=0;
  bool badInput=false;
  inputFile.open(structures_file);

  if (!inputFile.is_open()) {
    cout << "Could not open the grammar file\n";
    return false;
  }
  inputValue.pivotPoint=0;
  while (!inputFile.eof()) {
    badInput=false;
    getline(inputFile,inputLine);
    marker=inputLine.find("\t");
    if (marker!=string::npos) {
      prob=atof(inputLine.substr(marker+1,inputLine.size()).c_str());
      inputLine.resize(marker);
      inputValue.probability=prob;
      inputValue.base_probability=prob;
      pastCase='!';
      curSize=0;
      for (int i=0;i<inputLine.size();i++) {
        if (curSize==MAXWORDSIZE) {
          badInput=true;
          break;
        }
        if (pastCase=='!') { 
          pastCase=inputLine[i];
          curSize=1;
        }
        else if (pastCase==inputLine[i]) {
          curSize++;
        }
        else {
          if (pastCase=='L') {
            if (dicWords[curSize]==NULL) {
              badInput=true;
              break;
            }
            inputValue.replacement.push_back(dicWords[curSize]);
            inputValue.probability=inputValue.probability*dicWords[curSize]->probability;
          }
          else if (pastCase=='D') {
            if (numWords[curSize]==NULL) {
              badInput=true;
              break;
            }
            inputValue.replacement.push_back(numWords[curSize]);
            inputValue.probability=inputValue.probability*numWords[curSize]->probability;
          }
          else if (pastCase=='S') {
            if (specialWords[curSize]==NULL) {
              badInput=true;
              break;
            }
            inputValue.replacement.push_back(specialWords[curSize]);
            inputValue.probability=inputValue.probability*specialWords[curSize]->probability;
          }
          else {
            cout << "WTF Weird Error Occured\n";
            return false;
          }
          curSize=1;
          pastCase=inputLine[i];
        }
      }
      if (badInput==true) { //NOOP
      }
      else if (pastCase=='L') {
        if ((curSize>=MAXWORDSIZE)||(dicWords[curSize]==NULL)) {
          badInput=true;
        }
        else {
          inputValue.replacement.push_back(dicWords[curSize]);
          inputValue.probability=inputValue.probability*dicWords[curSize]->probability;
        }
      }
      else if (pastCase=='D') {
        if ((curSize>=MAXWORDSIZE)||(numWords[curSize]==NULL)) {
          badInput=true;
        }
        else {
          inputValue.replacement.push_back(numWords[curSize]);
          inputValue.probability=inputValue.probability*numWords[curSize]->probability;
        }
      }
      else if (pastCase=='S') {
        if ((curSize>=MAXWORDSIZE)||(specialWords[curSize]==NULL)) {
          badInput=true;
        }
        else {
          inputValue.replacement.push_back(specialWords[curSize]);
          inputValue.probability=inputValue.probability*specialWords[curSize]->probability;
        }
      }
      if (!badInput) {
        if (inputValue.probability==0) {
          cout << "Error, we are getting some values with 0 probability\n";
          return false;
        }
        pqueue->push(inputValue);
      }
      inputValue.probability=0;
      inputValue.replacement.clear();      
    }
  } 


  inputFile.close(); 
  return true;
}



bool generateGuesses(pqueueType *pqueue,bool isLimit,long long maxGuesses) {
  pqReplacementType curQueueItem;
  int returnStatus;
  string curGuess;
  while (!pqueue->empty()) {
    curQueueItem  = pqueue->top();
    pqueue->pop();
    curGuess.clear();
    returnStatus= createTerminal(&curQueueItem,isLimit, &maxGuesses,0,&curGuess);
    if (returnStatus==1) { //made the maximum number of guesses
      return true;
    }
    else if (returnStatus==-1) { //an error occured
      return false;
    }
    pushNewValues(pqueue,&curQueueItem);
  }
  return true;
}


int createTerminal(pqReplacementType *curQueueItem,bool isLimit,long long  *maxGuesses, int workingSection, string *curOutput) {
//  string guessOutput;
  list <string>::iterator it;
  int size = curOutput->size();
  for (it=curQueueItem->replacement[workingSection]->word.begin();it!=curQueueItem->replacement[workingSection]->word.end();++it) {
    curOutput->resize(size);
    curOutput->append(*it);
    if (workingSection==curQueueItem->replacement.size()-1) {

      cur_guesses += 1;
      fout << *curOutput << '\n';
      if (cur_guesses >= *maxGuesses) {
          end_time = clock();
          std::cout << "The speed is: " << std::fixed << cur_guesses / ((double)(end_time - start_time) / CLOCKS_PER_SEC)  << std::endl;
          std::exit(0);
      }
    }
    else {
      createTerminal (curQueueItem, isLimit, maxGuesses, workingSection+1, curOutput);
    }
  }

  return 0;
} 


bool pushNewValues(pqueueType *pqueue,pqReplacementType *curQueueItem) {
  pqReplacementType insertValue;

  insertValue.base_probability=curQueueItem->base_probability;
  for (int i=curQueueItem->pivotPoint;i<curQueueItem->replacement.size();i++) {
    if (curQueueItem->replacement[i]->next!=NULL) {
      insertValue.pivotPoint=i;
      insertValue.replacement.clear();
      insertValue.probability=curQueueItem->base_probability;
      for (int j=0;j<curQueueItem->replacement.size();j++) {
        if (j!=i) {
          insertValue.replacement.push_back(curQueueItem->replacement[j]);
          insertValue.probability=insertValue.probability*curQueueItem->replacement[j]->probability;
        }
        else {
          insertValue.replacement.push_back(curQueueItem->replacement[j]->next);
          insertValue.probability=insertValue.probability*curQueueItem->replacement[j]->next->probability;
        }
      }
      pqueue->push(insertValue); 
    }
  }
  return true;
}
