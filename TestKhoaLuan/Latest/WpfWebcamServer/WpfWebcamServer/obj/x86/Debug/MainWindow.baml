   M S B A M L   `   `   `  ����M  IFluent, Version=1.3.0.0, Culture=neutral, PublicKeyToken=3e436e32a8c5546f/$clr-namespace:Fluent;assembly=FluentFluent  5   	    WpfWebcamServer    WpfWebcamServer.MainWindow   5      \ XPresentationFramework, Version=4.0.0.0, Culture=neutral, PublicKeyToken=31bf3856ad364e35W SPresentationCore, Version=4.0.0.0, Culture=neutral, PublicKeyToken=31bf3856ad364e35R NWindowsBase, Version=4.0.0.0, Culture=neutral, PublicKeyToken=31bf3856ad364e35D 9http://schemas.microsoft.com/winfx/2006/xaml/presentation    5   	   R NSystem.Xaml, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b77a5c561934e089:x,http://schemas.microsoft.com/winfx/2006/xaml     5   	   2r$clr-namespace:Fluent;assembly=Fluent     5   	   -   5          � Title$  Webcam Server��5   	   $	��640��6   $
��1024��6,   $	��640��69   $	��900��6I    � WindowStartupLocation$ CenterScreen=�6o   .��5      �  � RowDefinitions 5   
   �� 5      $
��Auto �6   �� 5	      $��* �6   .+�5   
      Fluent.Ribbon  $��0��6   $��0��6    $��2��60      Tabs. 5         Fluent.RibbonTabItem     Header$
 Home��6      Groups. 5         Fluent.RibbonGroupBox     Header$ Settings��6#   .@�5         Fluent.Button  -   5           Fluent.RibbonControl   Text$ Network��5         	LargeIcon$ Images\network.png��6.   	 � ToolTip$E	 ?Config the ports use to connect to webcam recorders and clients��6M   6�     5      -   5        $ 	Recording��5      $ Images\video_record.png��60   $P	 JAllow/disallow video recording and change location to store recorded files��6T   6�     5      -   5        $ Alarm��5      $ Images\sound_alarm.png��6,   $ 	 Allow/disallow sound alarm��6O   6t   5        5      $ Cameras��6#   .@�5      �  5      �� 5      $��* �6,   �� 5      $��* �6,   
 � ColumnDefinitions
 5      �� 5      $��* �6/   �� 5      $��* �6/   �� 5      $��* �6/   �� 5      $��* �6/   .+�5         -   5        $ Add Recorder��5    #   $:	 4Add a recorder manually by entering address and port��67   $ Images\add.png��6v   $��0��6�   $��0��6�   $��2��6�      Fluent.ComboBox  5!      -   5        $��	cbxDevice��5!   %   $	��180��66      
IsEditable .  $��0��6U   $��1��6b   $��2��6r   $��6q�6�   6�      Fluent.CheckBox  5"      -   5        $��	chkRecord��5"   %   $ Video Record��66   $*	 $Check to enable video record feature��6J   $��1��6�   $��1��6�   $
��6, 0q�6�     5#      -   5        $��chkSound��5#   %   $ Sound Alarm��66   $)	 #Check to enable sound alarm feature��6I   $��1��6�   $��2��6�     5$      -	   5        $ Auto Detect��5$   #   $;	 5Automatically detect all cameras in the local network��66   $ Images\auto_detect.png��6v   $��0��6�   $��3��6�   $��2��6�   5%      5&      5'      5(      � 5*   
   $��1��6   $��0��6   
 5+      �� 5,      $��* �6#   �� 5-      $	��250 �6#   .+�50      �� $��Disabled=�6   $
��Auto=�6D   $��0��6g   $��0��6t   .��51      � -
   5        $��	pnlWebcam��51      52      �� 53      -   5        $��
tabControl��53      $��0��6+   $��1��68   
������ $��0q�6a   .@�54      �� $
��Info��6   .��55      �  56      �� 57      $
��Auto �6,   �� 58      $��* �6,   .+�5;      �� $��0��6%   $��0��62   .+�5<      �� 
�������$��1q�6=    �� CornerRadius$ 5��6Q   $��
0, 5, 0, 5q�6b   ��5=   "   �� 5>   &   ��5@   .   � � 5A   2    � Color$ Black��6?    � Offset$	 0.0Q�6M   � 5B   2   $ DarkGray��6?   $
 0.75Q�6P   5C   /   5E   '   ._�5G   "   �� $��Clients��6,   
�������$
��Bold"�6N    �� Padding$ 5q�6`   5H      �� 5I      
���   �$��1q�69   $ 5��6M   ._�5J   "   �� -   5        $��	lstClient��5J   *   $	��120��6;   5K      �� 5L      
�������$��1q�6=   $ 5��6Q   $��
0, 5, 0, 5q�6b   ��5M   "   �� 5N   &   ��5P   .   � � 5Q   2   $ Black��6?   $	 0.0Q�6M   � 5R   2   $ DarkGray��6?   $
 0.75Q�6P   5S   /   5U   '   ._�5W   "   �� $��Status��6,   
�������$
��Bold"�6M   $ 5q�6_   5X      5Y      �� 5[      
���   �$��1q�65   $ 5��6I   $��1��6Z   $��0��6g   ._�5\      �� -   5        $��	lstStatus��5\   &   $	��260��67   $��5q�6G   5]      5^      5_      �� 5`      $��	Recording��6   .��5a      �� 
���   �$��1q�61   $ 5��6E   ._�5b      �� $
��Auto=�6'   $��Disabled=�6J   .��5c      � -   5        $��
videoPanel��5c   (   
�������6O   5d      5e      5f      5g      5h      5i      5j      