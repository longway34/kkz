/* 
 * File:   Filters.cpp
 * Author: longway
 * 
 * Created on 7 июля 2016 г., 9:17
 */

#include "Filters.h"

/*
 Класс для вычисления текущего среднего значения 
 */

void AvrSumm::Init(int _size){
    size = _size; index = 0; length = 0; summa = 0;
    if(size > 0){
        if(!summs){
            summs = (float*)malloc(size * sizeof(float));
        }
        memset(summs, 0, size * sizeof(float));
    }
}

float AvrSumm::addNext(float _s){
    if(length == size){
        summa -= summs[index];
    } else {
        length++;
    }
    summs[index] = _s; 
    summa = summa + _s;
    index++; if(index == size){ index = 0;}
    return summa;
}

/**********************************************************************/
/* Класс для вычисление СКО/Дисперсии классическим методом            */
/**********************************************************************/

void SigmaClassic::Init(int _size){
    AvrSumm::Init(_size); 
    sigma = 0; min = 0; max = 0; MO = 0;
}

void SigmaClassic::Init(SigmaClassic* orig){
    AvrSumm::Init(orig->size);
    memcpy(this->summs, orig->summs, sizeof(float) * size);
    this->MO = orig->MO;
    this->index = orig->index; this->length = orig->length; this->max = orig->max; this->min = orig->min; this->sigma = orig->sigma;
    this->summa = orig->summa;
}

float SigmaClassic::addNext(float _s){
    AvrSumm::addNext(_s);
    MO = summa / length;
    int count=0; float S = 0;
    count = 0;
    while(count < length){
        float s = (summs[count] - MO);
        S += s*s;
        count++;
    }
    sigma = sqrt(S / length);
    max = MO + sigma * 3;
    min = MO - sigma * 3;
    return sigma;
}

void SigmaClassicFilter::Init(int _size){
    AvrSumm::Init(_size);
    if(summs2) delete summs2;
    if(!summs2) summs2 = new AvrSumm(_size);
}

float fabs(float arg){ return arg > 0 ? arg : -1 * arg; }

float SigmaClassicFilter::addNextToFilter(float _s){

    float res = _s;
//    SigmaClassic::addNext(_s);
//    summs2->addNext(_s * _s);
    
    if(length >= size){
        float variance = (summs2->summa - (this->summa * this->summa)/length) / (length - 1);
        float sigma = sqrt(fabs(variance));
        float _MO = getAvr();
        float delta = fabs(_s-_MO);
        if(delta > sigma * 4){
            sign++;
            if(sign & 0x01){
//            if(MO > _s){
                res = _MO - sigma * 1.1;
            } else {
                res = _MO + sigma * 1.1;
            }
        } else {
            sign = 0;
        }
    }
    AvrSumm::addNext(res);
    summs2->addNext(res * res);
    return res;
}

/**************************************************************************/
/* Класс для вычисление СКО/Дисперсии скользящим методом (Ачинск 06.2016) */
/**************************************************************************/
void SigmaSlide::Init(float _mo_begin, float _disp_begin, float _k_mo, float _k_disp, float _k_sigma){
    if(_k_disp < 0) K_Disp = DEF_K_DISP; else K_Disp = _k_disp;
    K_Disp_min = 1 - K_Disp;
    if(_k_mo < 0) K_MO = DEF_K_MO; else K_MO = _k_mo;
    K_MO_min = 1 - K_MO;
    K_Sigma = _k_sigma;

    if(_mo_begin < 0) mo = 0; else mo = _mo_begin;
    if(_disp_begin < 0) disp = mo * 3; else disp = _disp_begin;

    sigma = sqrt(disp);
    signalMin = mo - sigma * K_Sigma; 
    signalMax = mo + sigma * K_Sigma;
}

float SigmaSlide::addNext(float _next){
    mo = (mo * K_MO) + (_next * K_MO_min);
    disp = (disp * K_Disp) + (K_Disp_min * (mo - _next)*(mo - _next));
    sigma = sqrt(disp);
    signalMin = mo - sigma * K_Sigma;
    signalMax = mo + sigma * K_Sigma;
    return sigma;
}
/*************** Class ShiftAvrSumm **************************/

float ShiftAvrSumm::addNext(float _s){
    if(length < size){
        AvrSumm::addNext(_s);
    } else {
        summa = summa - summs[0];
        memcpy(summs, &summs[1], sizeof(float) * (length-1));
        summs[length-1] = _s;
        summa += _s;
    }
    return summa;
}

/*************** Class AParabola *****************************/

AParabola::AParabola(int _channel, int _ratio, SummsRatiosData *_smData, int _min, int _smDataLength) {
//    memset(&params, 0, sizeof(ev_parabols));
//    params.a = (float*)malloc(sizeof(float) * 3);
    Init(_min, _channel, _ratio, _smData, _smDataLength);
}

AParabola::~AParabola(){
   if(params.a) free(params.a); 
}

void AParabola::Init(int _min, int _channel, int _ratio, SummsRatiosData *_smData, int _smDataLength){
    if(_channel >= 0){ channel = _channel; }
    if(_ratio >= 0){ ratio = _ratio; }
    this->params.Init();
    beginIndex = -1;
    Y_source.clear();
    X_source.clear();
    length = 0;
    if(_smData){ smData = _smData; } 
    else { // нет ссылки на шаред мемори используем внутренний вектор
        
    }
    if(_min > 0){ minLength = _min; }
    if(_smDataLength > 0){ smDataLength = _smDataLength; }
    SX = 0; SX2 = 0; SX3 = 0; SX4 = 0;
    SY = 0; SXY = 0; SX2Y = 0; 
}

ev_parabols *AParabola::addNext(float Y, int X, int _index){
    if(beginIndex < 0) beginIndex = _index;
    float x2 = X * X;
    SX += X; SX2 += x2; SX3 += x2*X; SX4 += x2 * x2;
    SY += Y, SXY += X * Y; SX2Y += x2*Y;
    if(!smData){
        Y_source.push_back(Y);
        X_source.push_back(X);
    }
    
    length++;
    if(length >= minLength){
       return complite(); 
    }
    return &params;
}

ev_parabols *AParabola::complite(){
        float D[3][3] = {{SX2,      SX,     (float)length},
                         {SX3,      SX2,    SX},
                         {SX4,      SX3,    SX2}};
        float d = getDeterminant(D);
        float DA[3][3] = {{SY,   SX,        (float)length},
                          {SXY,     SX2,    SX},
                          {SX2Y,    SX3,    SX2}};
        float da = getDeterminant(DA);
        params.a[2] = da / d;
        
        float DB[3][3] = {{SX2,     SY,     (float)length},
                          {SX3,     SXY,    SX},
                          {SX4,     SX2Y,   SX2}};
        float db = getDeterminant(DB);
        params.a[1] = db / d;
        
        float DC[3][3] = {{SX2,     SX,     SY},
                          {SX3,     SX2,    SXY},
                          {SX4,     SX3,    SX2Y}};
        float dc = getDeterminant(DC);
        params.a[0] = dc / d;
    
        int i=0; int index = beginIndex;
        float s_disp = 0, s_covar = 0;
        float MO = SY / length;
        while(i++ < length){
            float Y; int X = i;
            if(smData){
                Y = smData[index].signalSigma[channel][ratio];
                index++;
                if(index >= smDataLength) index = 0;
            } else {
                Y = Y_source.at(index);
                X = X_source.at(index);
                index++;
                if(index >= Y_source.size()){
                    index = 0;
                }
            }
            
            float disp = MO - Y; 
            s_disp += disp * disp;
            float YN = params.a[2]*X*X + params.a[1]*X + params.a[0];
            float covar = YN - Y;
            s_covar += covar * covar;
        }
        params.R2 = 1 - s_covar / s_disp;
        return &params;
}

float AParabola::getDeterminant(float B[3][3]){
        float p = (B[0][0]*B[1][1]*B[2][2]) + (B[0][1]*B[1][2]*B[2][0]) + (B[0][2]*B[1][0]*B[2][1]);
        float m = -1 * ((B[0][2]*B[1][1]*B[2][0])+(B[0][0]*B[1][2]*B[2][1])+(B[0][1]*B[1][0]*B[2][2]));
        return p + m;
};

/*************** Class Parabola ******************************/
float Parabola::getDeterminant(float B[3][3]){
        float p = (B[0][0]*B[1][1]*B[2][2]) + (B[0][1]*B[1][2]*B[2][0]) + (B[0][2]*B[1][0]*B[2][1]);
        float m = -1 * ((B[0][2]*B[1][1]*B[2][0])+(B[0][0]*B[1][2]*B[2][1])+(B[0][1]*B[1][0]*B[2][2]));
        return p + m;
};

ev_parabols *Parabola::complite(int type){
        float SX = 0, SX2 = 0, SX3 = 0, SX4 = 0; 
        float SXY = 0, SX2Y = 0;
        
        for(int i=0; i<source->length; i++){
            float x = type > 0 ? X->getSumms()[i] : i;
//            float x = i;
            float x2 = x * x; float x3 = x2 * x; float x4 = x2 * x2;
            float xy = x * source->getSumms()[i]; float x2y = x2 * source->getSumms()[i];
            
            SX += x; SX2 += x2; SX3 += x3; SX4 += x4;
            SXY += xy; SX2Y += x2y;
            
        }
        float D[3][3] = {{SX2,      SX,     (float)source->length},
                         {SX3,      SX2,    SX},
                         {SX4,      SX3,    SX2}};
        float d = getDeterminant(D);
        float DA[3][3] = {{source->summa,   SX,     (float)source->length},
                          {SXY,     SX2,    SX},
                          {SX2Y,    SX3,    SX2}};
        float da = getDeterminant(DA);
        params.a[2] = da / d;
        
        float DB[3][3] = {{SX2,     source->summa,  (float)source->length},
                          {SX3,     SXY,    SX},
                          {SX4,     SX2Y,   SX2}};
        float db = getDeterminant(DB);
        params.a[1] = db / d;
        
        float DC[3][3] = {{SX2,     SX,     source->summa},
                          {SX3,     SX2,    SXY},
                          {SX4,     SX3,    SX2Y}};
        float dc = getDeterminant(DC);
        params.a[0] = dc / d;
        
        float s2 = 0;
        for(int x=0; x < source->length; x++){
//            float f0 = params.a * X->getSumms()[x] * X->getSumms()[x] + params.b * X->getSumms()[x] + params.c;
            float f0 = params.a[0] * x * x + params.a[1] * x + params.a[2];
            func->addNext(f0);
            float diff = (source->getSumms()[x] - f0);
            s2 += diff * diff;
            params.x0 = -1*(params.a[1] / (2 * params.a[2]));
        }
        params.R2 = sqrt(s2);
        return &params;
}
/******************************** Парабола 3 степени ********************************/
ev_parabols *Parabola3::complite_1(int type){
//    // заполняем квадратную матрицу сумм степеней X (summ_x)
//    if(source->length >= K){
//        for(int i=0; i<K+1; i++){
//            for(int j=0; j<K+1; j++){
//                summ_x[i][j] = 0;
//                for(int x=0; x<source->length; x++){
//                    float fx = type > 0 ? X->getSumms()[x] : x;
//                    summ_x[i][j] += pow(fx, i+j);
//                }
//            }
//        }
//        // заполняем массив свободных членов (summ_xy)
//        for(int i=0; i<K+1; i++){
//            summ_xy[i] = 0;
//            for(int x=0; x < source->length; x++){
//                float fx = type > 0 ? X->getSumms()[x] : x;
//                summ_xy[i] += pow(fx, i) * source->getSumms()[x]; 
//            }
//        }
//        // убираем нули с главной диагонали если они там есть
//        float temp=0;
//        for(int i=0; i<K+1; i++){
//            if(summ_x[i][i] == 0){
//                for(int j=0; j<K+1; j++){
//                    if(i == j) continue;
//                    if(summ_x[j][i] != 0 && summ_x[i][j] != 0){
//                        for(int k=0; k<K+1;k++){
//                            temp = summ_x[j][k];
//                            summ_x[j][k] = summ_x[i][k];
//                            summ_x[i][k] = temp;
//                        }
//                        temp = summ_xy[j];
//                        summ_xy[j] = summ_xy[i];
//                        summ_xy[i] = temp;
//                        break;
//                    }
//                }
//            }
//        }
////        printMatrix();
//        for(int k=0; k<K+1; k++){
//            for(int i=k+1; i<K+1; i++){
//                if(summ_x[k][k] == 0){
//                    fprintf(stdout, "\nSolution is not exist.\n");
//                    return NULL;
//                }
//                float M = summ_x[i][k] / summ_x[k][k];
//                for(int j=k; j<K+1; j++){
//                    summ_x[i][j] -= M * summ_x[k][j];
//                }
//                summ_xy[i] -= M*summ_xy[k];
//            }
//        }
//        for(int i=(K+1)-1; i>=0; i--){
//            float s=0;
//            for(int j=i; j<K+1; j++){
//                s += summ_x[i][j] * params.a[j];
//            }
//            params.a[i] = (summ_xy[i] - s) / summ_x[i][i];
//        }
////        printMatrix();
//        // находим отклонения
//        float disp = 0;
//        for(int i=0; i<source->length; i++){
//            float x = type > 0 ? X->getSumms()[i] : i;
//            float fy = 0;
//            for(int j=K; j>=0; j--){
//                fy += params.a[j] * pow(x, j);
//            }
//            disp += pow(fabs(fy - source->getSumms()[i]), 2);
//        }
//        params.sq = sqrt(disp);
//        params.x0 = (-1 * (params.a[K-1] * 2))/(params.a[K] * 6);
//    }
//    return &params;
}

int Parabola3::normMatrix(int nStr){
//    printMatrix();
    for(int i=nStr; i<K+1; i++){ // i = строка нормируем
       float normStr = summ_x[i][nStr]; 
       for(int j=nStr; j<K+1; j++){ // столбец
           summ_x[i][j] = summ_x[i][j] / normStr;
       }
       summ_xy[i] = summ_xy[i] / normStr;
    }
//    printMatrix();
    for(int i=nStr+1; i<K+1; i++){ // i - string вычитаем первую строку из остальных
        for(int j=nStr; j<K+1; j++){ // j - collumn
            summ_x[i][j] = summ_x[i][j] - summ_x[nStr][j];
        }
        summ_xy[i] = summ_xy[i] - summ_xy[nStr];
    }
    return nStr;
}



ev_parabols *Parabola3::complite(int type){
    // заполняем квадратную матрицу сумм степеней X (summ_x)
    if(source->length > K){
        for(int i=0; i<K+1; i++){
            for(int j=0; j<K+1; j++){
                summ_x[i][j] = 0;
                for(int x=0; x<source->length; x++){
                    float fx = type > 0 ? X->getSumms()[x] : x;
                    summ_x[i][j] += pow(fx, i+j);
                }
            }
        }
        // заполняем массив свободных членов (summ_xy)
        for(int i=0; i<K+1; i++){
            summ_xy[i] = 0;
            for(int x=0; x < source->length; x++){
                float fx = type > 0 ? X->getSumms()[x] : x;
                summ_xy[i] += pow(fx, i) * source->getSumms()[x]; 
            }
        }
        
        for(int nStr=0; nStr < K+1; nStr++){
            normMatrix(nStr);
        }
        
        for(int nStr=K+1; nStr >=0; nStr--){
            compResults(nStr);
        }

        
        float disp = 0;
        for(int x=0; x < source->length; x++){
            float fx = type > 0 ? X->getSumms()[x] : x;
            float fy = 0;
            for(int i=0; i<K+1; i++){
                fy += params.a[i] * pow(fx, i);
            }
            disp += pow((fy - source->getSumms()[x]), 2);
        }
        params.R2 = sqrt(disp);

        params.x0 = (-2 * params.a[2]) / (6 * params.a[3]);
        
//        printf("Total: %+f*x3 %+f*x2 %+fx %+f\n", params.a[3], params.a[2], params.a[1], params.a[0]);
//        printf("X0 %f sq %f\n", params.x0, params.sq);
    }    
}

int Parabola3::compResults(int nStr){
    float summ = summ_xy[nStr];
    for(int j=K+1; j>nStr; j--){
        summ -= params.a[j] * summ_x[nStr][j];
    }
    params.a[nStr] = summ;
}