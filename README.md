# Комплекс конвейерной защиты.

Комплекс конвейерной защиты (металлодитектор в простонародье) разрабатывался инженерами ООО «Технорос» для установки на транспортных коммуникациях (в основном на конвейерах) промышленных компаний для обнаружения засоряющих элементов и выдачи управляющих сигналов на устройства извлечения или останова конвейера для организации ручного извлечения найденных фрагментов.
Засоряющими элементами являются металлические куски выше устанавливаемого размера. Это могут быть металлические куски как из т. н. магнитных, слабо магнитных и немагнитных сталей и сплавов. Последние особенно опасны, т. к. не реагируют на оборудование металоизвлечения предприятия, работающих на электромагнитном принципе.
Устройство таким образом предназначено для предотвращения:
* попадания в устройства измельчения (дробилки среднего и мелкого дробления) и, как следствие, избегать их поломок и дорогостоящего ремонта;
* порчи и/или сбоев в работе конвейерного оборудования (обрывы и/или порезы контейнерной ленты, засорение пересыпных бункеров, шиберов и т. п.)
* прокламаций со стороны покупателей о поставках некондиционного сырья и, как следствие аварий на оборудовании партнеров, вызванных наличием засоряющих элементов в поставляемом сырье.

## Состав оборудования.

* Генератор прямоугольного электромагнитного сигнала в том числе передающей катушки излучения, монтируемой над транспортной (конвейерной) лентой, разработки ООО «Технорос».
* Комплекс приемных электромагнитных катушек, оснащенных предварительным усилителем, располагающихся под транспортной (конвейерной) лентой, разработки ООО «Технорос»
* Основной вычислительный комплекс, выполненный на базе Одноплатного компьютера формата PC/104-Plus Advantech PCM-3362
![Advantech PCM-3362](https://raw.githubusercontent.com/longway34/kkz/main/Common/demo_images/DM-7520.pngб "Advantech PCM-3362")

в комплекте с АЦП RTD Embedded Technologies, Inc. DM-7520
![RTD Embedded Technologies, Inc. DM-7520](https://raw.githubusercontent.com/longway34/kkz/main/Common/demo_images/PCM-3362.png "RTD Embedded Technologies, Inc. DM-7520")

---

* Устройство защиты оборудования от прохождения негабаритных фрагментов под управлением программируемого реле ОВЕН ПР-200
![ОВЕН ПР-200](https://raw.githubusercontent.com/longway34/kkz/main/Common/demo_images/PR-200.png "ОВЕН ПР-200")

связь между ПР-200 и вычислительным комплексом организована по протоколу Modbus-RTU RS-485. Устройство защиты в своем составе снабжено набором датчиков движения ленты, прохождения негабарита, аварийным подъемным устройством, набором концевых выключателей, и т. п. ПР-200 дополнительно использовался для:
* управления т. н. светоуказательной рампой при организации ручной выборки обнаруженных фрагментов;
* включения в систему АСУТП, выступая в роли Mobsus-источника данных.

---

