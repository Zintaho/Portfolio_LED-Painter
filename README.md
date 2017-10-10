# Portfolio_LED-Painter
LED Painter (Bluetooth 8x8 LED Matrix Painter) 프로젝트의 주요 소스 코드입니다.

-전체 소스 코드는 아래 링크를 참조해주십시오.
https://github.com/Zintaho/LED-Painter

-프로젝트 개요는 아래 링크를 참조해주십시오.
https://docs.google.com/document/d/1nvYZRokear7IOJqa_CGVC59Riwz-X4jK7mNaBL56fus/edit?usp=sharing

※Objective-C (X) C (O)

[C (Atmega128) 폴더] -- Atmega128 기판에서 사용된 소스코드 입니다.


main.c : 주 구현 소스 코드입니다.

pictures.h : LED Matrix 색상정보 프리셋이 있는 헤더입니다.

colors.h : 자주 사용되는 색상에 대한 헤더입니다.


[Java (Android) 폴더] -- Android 에서 사용된 소스코드 입니다.

MainActivity.java : 메인 액티비티 입니다.

GridAdapter.java : 그림을 그릴 8x8 그리드 어댑터입니다.


