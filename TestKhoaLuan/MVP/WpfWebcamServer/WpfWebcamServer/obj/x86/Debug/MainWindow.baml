   M S B A M L   `   `   `  ����M  IFluent, Version=1.3.0.0, Culture=neutral, PublicKeyToken=3e436e32a8c5546f/$clr-namespace:Fluent;assembly=FluentFluent  5   	    WpfWebcamServer    WpfWebcamServer.MainWindow   5      \ XPresentationFramework, Version=4.0.0.0, Culture=neutral, PublicKeyToken=31bf3856ad364e35W SPresentationCore, Version=4.0.0.0, Culture=neutral, PublicKeyToken=31bf3856ad364e35R NWindowsBase, Version=4.0.0.0, Culture=neutral, PublicKeyToken=31bf3856ad364e35D 9http://schemas.microsoft.com/winfx/2006/xaml/presentation    5   	   R NSystem.Xaml, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b77a5c561934e089:x,http://schemas.microsoft.com/winfx/2006/xaml     5   	   2r$clr-namespace:Fluent;assembly=Fluent     5   	   -   5          � Title$  Webcam Server��5   	   $	��640��6   $
��1024��6,   $	��640��69   $	��900��6I    � WindowStartupLocation$ CenterScreen=�6o   .��5      �  � RowDefinitions 5   
   �� 5      $
��Auto �6   �� 5	      $��* �6   .+�5   
      Fluent.Ribbon  $��0��6   $��0��6    $��2��60      Tabs. 5         Fluent.RibbonTabItem     Header$
 Home��6      Groups. 5         Fluent.RibbonGroupBox     Header$ Settings��6#   .@�5         Fluent.Button  -   5           Fluent.RibbonControl   Text$ Network��5         	LargeIcon$ Images\network.png��6.   	 � ToolTip$E	 ?Config the ports use to connect to webcam recorders and clients��6M   6�     5      -   5        $ 	Recording��5      $ Images\video_record.png��60   $P	 JAllow/disallow video recording and change location to store recorded files��6T   6�     5      -   5        $ Alarm��5      $ Images\sound_alarm.png��6,   $ 	 Allow/disallow sound alarm��6O   6t   5        5      $ Cameras��6#   .@�5      �  5      �� 5      $��* �6,   �� 5      $��* �6,   
 � ColumnDefinitions
 5      �� 5      $��* �6/   �� 5      $��* �6/   �� 5      $��* �6/   .+�5         Fluent.ComboBox  -   5        $��	cbxDevice��5   %   $	��180��66      
IsEditable .  $��0��6U   $��0��6b   $��2��6r   $��6q�6�   6�      Fluent.CheckBox  5       -   5        $��	chkRecord��5    %   $ Video Record��66   $*	 $Check to enable video record feature��6J   $��1��6�   $��0��6�   $
��6, 0q�6�     5!      -   5        $��chkSound��5!   %   $ Sound Alarm��66   $)	 #Check to enable sound alarm feature��6I   $��1��6�   $��1��6�     5"      -   5        $ Auto Detect��5"   #   $;	 5Automatically detect all cameras in the local network��66   $ Images\auto_detect.png��6v   $��0��6�   $��2��6�   $��2��6�   5#      5$      5%      5&      � 5(   
   $��1��6   $��0��6   
 5)      �� 5*      $��* �6#   �� 5+      $	��250 �6#   .+�5.      �� $��Disabled=�6   $
��Auto=�6D   $��0��6g   $��0��6t   .��5/      � -	   5        $��	pnlWebcam��5/      50      �� 51      -
   5        $��
tabControl��51      $��1��6+   $��1��68   
������ $��0q�6a   .@�52      �� $
��Info��6   .��53      �� .+�54      �� 
�������$��1q�69    �� CornerRadius$ 5��6M   $��
0, 5, 0, 5q�6^   ��55      �� 56   "   ��58   *   � � 59   .    � Color$ Black��6;    � Offset$	 0.0Q�6I   � 5:   .   $ DarkGray��6;   $
 0.75Q�6L   5;   +   5=   #   ._�5?      �� $��Clients��6(   
�������$
��Bold"�6J    �� Padding$ 5q�6\   5@      �� 5A      
���   �$��1q�65   $ 5��6I   ._�5B      �� -   5        $��	lstClient��5B   &   $	��120��67   5C      �� 5D      
�������$��1q�69   $ 5��6M   $��
0, 5, 0, 5q�6^   ��5E      �� 5F   "   ��5H   *   � � 5I   .   $ Black��6;   $	 0.0Q�6I   � 5J   .   $ DarkGray��6;   $
 0.75Q�6L   5K   +   5M   #   ._�5O      �� $��Status��6(   
�������$
��Bold"�6I   $ 5q�6[   5P      �� 5Q      
���   �$��1q�65   $ 5��6I   ._�5R      �� -   5        $��	lstStatus��5R   &   $	��260��67   $��5q�6G   5S      5T      5U      �� 5V      $��	Recording��6   .��5W      �� 
���   �$��1q�61   $ 5��6E   ._�5X      �� $
��Auto=�6'   $��Disabled=�6J   .��5Y      � -   5        $��
videoPanel��5Y   (   
�������6O   5Z      5[      5\      5]      5^      5_      5`      