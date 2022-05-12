# Lane & Object Detection

<p align="center"><img src = "https://github.com/junhyukch7/Lane-and-Object-Detection/blob/main/Result/Lane%20Object.gif" width="50%">
  <p align="center">Lane and Object Detection using OpenCV and MobileNet

## PipeLine
1.	보정된 이미지의 관점(Bird’s eye)을 변환하여, 이미지의 원근감이 없도록 변환.
2.	전처리 과정을 통해, 색을 필터링과 binary이미지로 변환.
3.	차선 인식을 위해 필요한 영역만 Masking.
4.	이미지의 크기를 반으로 잘라, 차선이 있을만한 영역의 x좌표들을 구함. 
5.	Sliding Window를 돌려가며, 차선의 영역의 점들을 저장.
6.	획득한 차선의 좌표 정보를 선형 방정식으로 보정하여 차선의 영역을 추정.
7.	추정된 차선들의 꼭짓점을 이용하여, Lane overlay영역을 결정.

## 2. Algorithm Detail
2.1 birdeye() – LaneDetector Class
    
- Warping 하기 전의 좌표와 Warping 한 후의 좌표를 설정하고. 변환한 후의 행렬과 역행렬을 구함. 역행렬을 구하는 이유는 추후 이미지를 원래의 이미지로 되돌리기 위해 필요. 행렬은 Homography Matrix를 의미하고 이는 하나의 평면을 다른 평면으로 투시변환하는 것과 같은 관계를 가짐. 이를 통해 3차원 평면의 차선을 2차원 평면으로 변환.

2.2 filter_color() & preprocess()- LaneDetector Class

- HSV 이미지의 경우, Hue가 일정한 범위를 갖는 순수한 색 정보를 갖기 때문에 RGB보다 쉽게 색을 분류할 수 있기 때문에 변환한 후 색깔을 필터링을 진행. 이후 연산량을 줄이고 차선의 영역을 두드러지게 하기 위해 임계값을 기준으로 기준치 미만의 경우 검은색, 이상인 경우 흰색으로 표현하여 차선을 표시.
    
2.3 Roi() - LaneDetector Class
    
- roi를 지정하고자 하는 4개의 꼭지점을 지정하고 해당 영역을 생성한다. 이후 masking 영역과 이미지를 and연산하여 겹치는 영역인 차선을 masking한다.
    
2.4 HalfDwinsizing() & SumCol() & getXbase() - LaneDetector Class
    
- 차선의 x좌표를 알기 위해서 이진화 된 이미지의 열성분을 다 더하여 가장 큰 값의 좌표 위치를 찾는다. Sliding Window가 시작될 첫 x좌표만 알면 되기 때문에 이미지의 크기를 절반으로 잘라, 열성분을 다 더한다. 이후 열벡터들의 합을 이용하여 좌표를 찾는다.
이미지의 열합 벡터를 1/4로 잘라 차선의 왼쪽 영역과 오른쪽 영역을 구분한다. 픽셀의 1/4부터 2/4영역까지는 왼쪽차선의 영역이고, 2/4부터 3/4까지는 오른쪽 차선의 영역이다. 이후 각 영역에서 최대값의 좌표위치를 구하면 이것이 차선이 위치하는 x좌표의 위치가 된다.
    
2.5 Sliding Windows() - LaneDetector Class
    
- 앞서 구한 x좌표를 기반으로 window의 좌표를 지정해준다. 이후 해당 영역에서 findNonZero함수를 이용하여 0이 아닌 픽셀의 벡터를 반환한다. 이 벡터들의 x좌표를 이용하여 평균 x좌표를 구하면, 최종적으로 차선의 x,y좌표를 얻을 수 있다. 이후를 y좌표를 이동하면서 최종적으로 정확한 차선의 좌표 위치들을 구하게 된다.
    
2.6 DrawingLine() - LaneDetector Class
 
- Sliding window를 통해 얻은 좌표들을 역투시변환하여 원래 이미지에서 위치할 차선의 좌표들을 구한다. 이 좌표들을 기반으로 기울기를 구하여, y좌표가 이미지의 높이 일 때 x좌표를 추정한다. 이는 직선의 방정식을 활용한 것으로, 점이 하나라도 있다면, 차선이 중간중간 끊기는 영역까지도 차선을 추정할 수 있다.
    
2.7 FinalDrawing() - LaneDetector Class

- drawingLine함수에서 얻은 좌표들을 이용하여 overlay point를 결정하고, 해당 영역을 마스킹한다. 이 영역이 실제 이미지에서 차선에 해당하는 영역이 된다. 
