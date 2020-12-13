/* 
 * File:   Filters.h
 * Author: longway
 *
 * Created on 7 июля 2016 г., 9:17
 */

#ifndef FILTERS_H
#define	FILTERS_H

#include "../Common/CommonObjects.h"

/*
 Класс для вычисления текущего среднего значения 
 */
class AvrSumm {
protected:
    float *summs;                       // массив предыдущих length элементов
public:
    int index;                          // индекс текущего/последнего элкмента
    float summa;                        // текущая сумма length элементов
    int length;                         // текущее значение длины массива
    int size;                           // максимальное значение массива

    AvrSumm(int _size) : summs(NULL){ Init(_size); }
    virtual ~AvrSumm() {if(summs) free(summs); summs = NULL;}

    virtual void Init(int _size);
    virtual float addNext(float _s);    // добавить следующий элемент для расчета
    inline virtual float getAvr(){ return ((float)summa) / ((float)(length));}      // получить среднее значение
    inline float *getSumms(){return summs;} // получить текущую сумму
    virtual void clear(){
        memset(summs,0,sizeof(float) * size);
        length = 0;
        summa = 0;
        index =0;
    }
private:

};

class ShiftAvrSumm : public AvrSumm{
public:
    ShiftAvrSumm(int _size) : AvrSumm(_size){};
    virtual float addNext(float _s); // добавить элемент сдвигом
};

/**********************************************************************/
/* Класс для вычисление СКО/Дисперсии классическим методом            */
/**********************************************************************/

class SigmaClassic : public AvrSumm{
protected:    
    float sigma, min, max;
    float MO;
public:
    SigmaClassic(int _size) : AvrSumm(_size){ Init(_size); };
    void Init(int _size);
    void Init(SigmaClassic *origin);
    virtual float addNext(float _s);
    float getSU_2(float _s){ float ret = (_s - MO); return ret * ret; }
    inline int getSize(){return size;}
    inline float getSigma(){return sigma; }
    inline float getMO(){return MO;}
    inline float get3SigmaMax(){ return max; }
    inline float get3SigmaMin(){ return min; }
};

class SigmaClassicFilter : public AvrSumm{
protected:
    AvrSumm *summs2;
    long sign;
    float num_sigma;
public:
    SigmaClassicFilter(int _size, float _num_sigma) : AvrSumm(_size), num_sigma(_num_sigma), sign(0), summs2(NULL){ Init(_size);}
    virtual ~SigmaClassicFilter(){if(summs2) delete summs2;};
    void Init(int _size);
    virtual float addNextToFilter(float _s);
    float getSumms2(){return summs2->summa;}
    void clear(){
        AvrSumm::clear();
        sign = 0;
       
    }
};


/**************************************************************************/
/* Класс для вычисление СКО/Дисперсии скользящим методом (Ачинск 06.2016) */
/**************************************************************************/

class SigmaSlide {
    float K_MO, K_MO_min;           // коэффициенты для приращения дисперсии 
    float K_Disp, K_Disp_min;       // коэффициенты для приращения мат. ожидания 
    float K_Sigma;                  // множитель сигма для определения пропуска итерации 
    float disp;                     // текущее значение дисперсии
public:    
    float sigma;                    // текущее значение СКО
    float mo;                       // текущее значение мат. ожидания
    float signalMin, signalMax;     // мин/макс знечение СКО для пропуска итерации

    SigmaSlide(float _k_mo = -1.0, float _k_disp = -1.0, float _mo_begin = -1, float _disp_begin = -1, float _k_sigma = 3){
        Init(_k_mo, _k_disp, _mo_begin, _disp_begin, _k_sigma);
    };
    
    void Init(float _mo_begin = -1, float _disp_begin = -1, float _k_mo = -1.0, float _k_disp = -1.0, float _k_sigma = 3);
    virtual float addNext(float _next);
    float fabs(float value){ if(value < 0) return -1 * value; else return value; };
    float getSU_Sigmas(float value){ return fabs(mo - value) / sigma; };

    float getSignalMax() const {return signalMax;}
    float getSignalMin() const {return signalMin;}
    float getMO() const {return mo;}
    float getSigma() const {return sigma;}
    float getDisp() const {return disp;}
    float getK_Sigma() const {return K_Sigma;}
    float getK_Disp_min() const {return K_Disp_min;}
    float getK_Disp() const {return K_Disp;}
    float getK_MO_min() const {return K_MO_min;}
    float getK_MO() const {return K_MO;}
};
/*****************************************************************************/
typedef struct _ev_parabols{
    float *a; // коэффициеты полинома начиная с младшей степени степени
    float x0;   // точка перегиба
//    float b;
//    float c;
    float R2;    // Коэффициент детерминации (добротность модели)
    int length;  // минимальная длина для вычисления параболы
    _ev_parabols(){
        a = NULL;
        Init();
    };
    void Init(){
        if(!a) { a = (float*)malloc(3 * sizeof(float)); }
        memset(a, 0, sizeof(float));
        x0 = 0;
        R2 = 0;
        length = 0;
    };
} ev_parabols;

class AParabola{
    ev_parabols params;
    int length;
    int beginIndex;
    SummsRatiosData *smData;
    int smDataLength;
    int minLength;  // минимальная длина для вычмсления параболы
    int channel;
    int ratio;
    std::vector<float> Y_source;
    std::vector<int> X_source;
    
    float SX, SX2, SX3, SX4;
    float SY, SXY, SX2Y;
    float getDeterminant(float B[3][3]);
public:
    AParabola(int _channel, int _ratio, SummsRatiosData *_smData, int _min=DEF_PAR_MIN, int _smDataLength=MAX_GRAPHIC_OBJECTS);
    virtual ~AParabola();
    void Init( int _min=-1, int _channel = -1, int _ratio = -1, SummsRatiosData *_smData = NULL, int _smDataLength=-1);
    
    ev_parabols *addNext(float Y, int X, int _index);
    ev_parabols *complite();
    ev_parabols *getParams(){ return &params; };
};

/***********************************************************************************/
class Parabola{
protected:
    ShiftAvrSumm *func;      // значения полученного палинома
    ShiftAvrSumm *source;    // исходные значения
    ShiftAvrSumm *X;         // аргументы

    ev_parabols params;
//    float a, b, c; // коэффициеты параболы
//    float sq;      // среднее квадратов уклонений от оригинала
    float getDeterminant(float B[3][3]);
//    float getSq();

public:
    
    Parabola(int _size, int _min = 5, int _K=2): source(NULL), func(NULL), X(NULL){
        K = _K;
        params.a = (float*)malloc(sizeof(float) * K+1);
        source = new ShiftAvrSumm(_size);
        func = new ShiftAvrSumm(_size);
        X = new ShiftAvrSumm(_size);
        Init(_size, _min);
    };
    virtual void Init(int _size, int _min = 5){
        if(_size != source->size){
            if(source) delete source;
            source = new ShiftAvrSumm(_size);
        }
        if(_size != func->size){
            if(func) delete func;
            func = new ShiftAvrSumm(_size);
        }
        if(_size != X->size){
            if(X) delete X;
            X = new ShiftAvrSumm(_size);
        }
        source->Init(_size);
        func->Init(_size);
        X->Init(_size);

        memset(params.a, 0, sizeof(float) * K+1); params.R2 = -1; params.length = _min;
    };

    virtual ev_parabols *complite(int type=0); // вычисления type =0 -> x0 = 0...; иначе x берется из X; и возврат значения sq

    float addNext(float arg, float _element){
        source->addNext(_element);
        X->addNext(arg);
        if(source->length >= params.length){
//        if(source->length == source->size){
            complite();
        }
        return params.R2;
    } 
    inline virtual float getA()  { return params.a[K]; }
    float getSq() { return params.R2;}
    ev_parabols *getParams(){ return &params; }
    inline int getLength(){ return source->length; }
    inline int getSize(){ return source->size; }
    inline virtual float getX0() {return params.x0; }
    
    virtual ~Parabola() {if(source) delete source; if(func) delete func; if(X) delete X; free(params.a); }
protected:
    int K; // степень полинома
};

class Parabola3 : public Parabola {
    float summ_x[4][4];
    float summ_xy[4];
//    float summ_x_def[4][4];          // суммы для typ==0
    
public:
    Parabola3(int _size, int _min=5, int _K=3) : Parabola(_size, _min, _K)/*, summ_x(NULL), summ_xy(NULL), summ_x_def(NULL)*/{
        Init(_size, _min);
    }
    virtual void Init(int _size, int _min = 5){
        Parabola::Init(_size, _min);
//        if(!summ_x) malloc(sizeof(float) * K + 1);
//        if(!summ_xy) malloc(sizeof(float) * K);
//        if(!summ_x_def) malloc(sizeof(float) * K + 1);
//        for(int i=0; i < K+1; i++){
//            summ_x[i] = malloc(sizeof(float));
//            summ_x_def[i] = malloc(sizeof(float));
//        }
    }
    ev_parabols *complite_1(int type=0);
    ev_parabols *complite(int type=0);
    void printMatrix(){
        printf("Iteration %d ", source->length);
        for(int i=0; i<source->length; i++){
            printf("%f\t", source->getSumms()[i]);
        }
        printf("*******************\n");
        for(int i=0; i< K+1; i++){
            printf("%d ", i);
            for(int j=0; j < K+1; j++){
                printf("%f\t", summ_x[i][j]);
            }
            printf("%f\n", summ_xy[i]);
        }
    }
    int normMatrix(int nStr);
    int compResults(int nStr);
};

#endif	/* FILTERS_H */

