﻿#pragma checksum "..\..\..\MainWindow.xaml" "{406ea660-64cf-4c82-b6f0-42d48172a799}" "B5103970086E99D07F1D391F94358360"
//------------------------------------------------------------------------------
// <auto-generated>
//     This code was generated by a tool.
//     Runtime Version:4.0.30319.1
//
//     Changes to this file may cause incorrect behavior and will be lost if
//     the code is regenerated.
// </auto-generated>
//------------------------------------------------------------------------------

using System;
using System.Diagnostics;
using System.Windows;
using System.Windows.Automation;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Markup;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Media.Effects;
using System.Windows.Media.Imaging;
using System.Windows.Media.Media3D;
using System.Windows.Media.TextFormatting;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Windows.Shell;


namespace WpfWebcamServer {
    
    
    /// <summary>
    /// MainWindow
    /// </summary>
    [System.CodeDom.Compiler.GeneratedCodeAttribute("PresentationBuildTasks", "4.0.0.0")]
    public partial class MainWindow : System.Windows.Window, System.Windows.Markup.IComponentConnector {
        
        
        #line 18 "..\..\..\MainWindow.xaml"
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Performance", "CA1823:AvoidUnusedPrivateFields")]
        internal System.Windows.Controls.Image pictureBox;
        
        #line default
        #line hidden
        
        
        #line 23 "..\..\..\MainWindow.xaml"
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Performance", "CA1823:AvoidUnusedPrivateFields")]
        internal System.Windows.Controls.Button autoDetectButton;
        
        #line default
        #line hidden
        
        
        #line 25 "..\..\..\MainWindow.xaml"
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Performance", "CA1823:AvoidUnusedPrivateFields")]
        internal System.Windows.Controls.ListBox deviceListBox;
        
        #line default
        #line hidden
        
        
        #line 27 "..\..\..\MainWindow.xaml"
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Performance", "CA1823:AvoidUnusedPrivateFields")]
        internal System.Windows.Controls.ListBox clientListBox;
        
        #line default
        #line hidden
        
        
        #line 31 "..\..\..\MainWindow.xaml"
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Performance", "CA1823:AvoidUnusedPrivateFields")]
        internal System.Windows.Controls.CheckBox recordCheckBox;
        
        #line default
        #line hidden
        
        
        #line 32 "..\..\..\MainWindow.xaml"
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Performance", "CA1823:AvoidUnusedPrivateFields")]
        internal System.Windows.Controls.TextBlock configureTextBlock;
        
        #line default
        #line hidden
        
        
        #line 35 "..\..\..\MainWindow.xaml"
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Performance", "CA1823:AvoidUnusedPrivateFields")]
        internal System.Windows.Controls.CheckBox soundCheckBox;
        
        #line default
        #line hidden
        
        
        #line 45 "..\..\..\MainWindow.xaml"
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Performance", "CA1823:AvoidUnusedPrivateFields")]
        internal System.Windows.Controls.TextBox broadcastTextBox;
        
        #line default
        #line hidden
        
        
        #line 46 "..\..\..\MainWindow.xaml"
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Performance", "CA1823:AvoidUnusedPrivateFields")]
        internal System.Windows.Controls.TextBox webcamTextBox;
        
        #line default
        #line hidden
        
        
        #line 47 "..\..\..\MainWindow.xaml"
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Performance", "CA1823:AvoidUnusedPrivateFields")]
        internal System.Windows.Controls.TextBox clientTextBox;
        
        #line default
        #line hidden
        
        
        #line 57 "..\..\..\MainWindow.xaml"
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Performance", "CA1823:AvoidUnusedPrivateFields")]
        internal System.Windows.Controls.ListBox statusListBox;
        
        #line default
        #line hidden
        
        
        #line 61 "..\..\..\MainWindow.xaml"
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Performance", "CA1823:AvoidUnusedPrivateFields")]
        internal System.Windows.Controls.TextBlock refreshTextBlock;
        
        #line default
        #line hidden
        
        
        #line 63 "..\..\..\MainWindow.xaml"
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Performance", "CA1823:AvoidUnusedPrivateFields")]
        internal System.Windows.Controls.WrapPanel videoPanel;
        
        #line default
        #line hidden
        
        private bool _contentLoaded;
        
        /// <summary>
        /// InitializeComponent
        /// </summary>
        [System.Diagnostics.DebuggerNonUserCodeAttribute()]
        public void InitializeComponent() {
            if (_contentLoaded) {
                return;
            }
            _contentLoaded = true;
            System.Uri resourceLocater = new System.Uri("/WpfWebcamServer;component/mainwindow.xaml", System.UriKind.Relative);
            
            #line 1 "..\..\..\MainWindow.xaml"
            System.Windows.Application.LoadComponent(this, resourceLocater);
            
            #line default
            #line hidden
        }
        
        [System.Diagnostics.DebuggerNonUserCodeAttribute()]
        [System.ComponentModel.EditorBrowsableAttribute(System.ComponentModel.EditorBrowsableState.Never)]
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Design", "CA1033:InterfaceMethodsShouldBeCallableByChildTypes")]
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Maintainability", "CA1502:AvoidExcessiveComplexity")]
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Performance", "CA1800:DoNotCastUnnecessarily")]
        void System.Windows.Markup.IComponentConnector.Connect(int connectionId, object target) {
            switch (connectionId)
            {
            case 1:
            
            #line 4 "..\..\..\MainWindow.xaml"
            ((WpfWebcamServer.MainWindow)(target)).Closed += new System.EventHandler(this.Window_Closed);
            
            #line default
            #line hidden
            return;
            case 2:
            this.pictureBox = ((System.Windows.Controls.Image)(target));
            return;
            case 3:
            this.autoDetectButton = ((System.Windows.Controls.Button)(target));
            
            #line 23 "..\..\..\MainWindow.xaml"
            this.autoDetectButton.Click += new System.Windows.RoutedEventHandler(this.autoDetectButton_Click);
            
            #line default
            #line hidden
            return;
            case 4:
            this.deviceListBox = ((System.Windows.Controls.ListBox)(target));
            
            #line 25 "..\..\..\MainWindow.xaml"
            this.deviceListBox.SelectionChanged += new System.Windows.Controls.SelectionChangedEventHandler(this.deviceListBox_SelectionChanged);
            
            #line default
            #line hidden
            return;
            case 5:
            this.clientListBox = ((System.Windows.Controls.ListBox)(target));
            return;
            case 6:
            this.recordCheckBox = ((System.Windows.Controls.CheckBox)(target));
            
            #line 31 "..\..\..\MainWindow.xaml"
            this.recordCheckBox.Checked += new System.Windows.RoutedEventHandler(this.recordCheckBox_Checked);
            
            #line default
            #line hidden
            
            #line 31 "..\..\..\MainWindow.xaml"
            this.recordCheckBox.Unchecked += new System.Windows.RoutedEventHandler(this.recordCheckBox_Checked);
            
            #line default
            #line hidden
            return;
            case 7:
            this.configureTextBlock = ((System.Windows.Controls.TextBlock)(target));
            
            #line 32 "..\..\..\MainWindow.xaml"
            this.configureTextBlock.MouseLeftButtonUp += new System.Windows.Input.MouseButtonEventHandler(this.configureTextBlock_MouseLeftButtonUp);
            
            #line default
            #line hidden
            return;
            case 8:
            this.soundCheckBox = ((System.Windows.Controls.CheckBox)(target));
            
            #line 35 "..\..\..\MainWindow.xaml"
            this.soundCheckBox.Checked += new System.Windows.RoutedEventHandler(this.soundCheckBox_Checked);
            
            #line default
            #line hidden
            
            #line 35 "..\..\..\MainWindow.xaml"
            this.soundCheckBox.Unchecked += new System.Windows.RoutedEventHandler(this.soundCheckBox_Checked);
            
            #line default
            #line hidden
            return;
            case 9:
            
            #line 36 "..\..\..\MainWindow.xaml"
            ((System.Windows.Controls.TextBlock)(target)).MouseLeftButtonUp += new System.Windows.Input.MouseButtonEventHandler(this.configureTextBlock_MouseLeftButtonUp);
            
            #line default
            #line hidden
            return;
            case 10:
            this.broadcastTextBox = ((System.Windows.Controls.TextBox)(target));
            return;
            case 11:
            this.webcamTextBox = ((System.Windows.Controls.TextBox)(target));
            return;
            case 12:
            this.clientTextBox = ((System.Windows.Controls.TextBox)(target));
            return;
            case 13:
            
            #line 48 "..\..\..\MainWindow.xaml"
            ((System.Windows.Controls.TextBlock)(target)).MouseLeftButtonUp += new System.Windows.Input.MouseButtonEventHandler(this.TextBlock_MouseLeftButtonUp);
            
            #line default
            #line hidden
            return;
            case 14:
            this.statusListBox = ((System.Windows.Controls.ListBox)(target));
            return;
            case 15:
            this.refreshTextBlock = ((System.Windows.Controls.TextBlock)(target));
            
            #line 61 "..\..\..\MainWindow.xaml"
            this.refreshTextBlock.MouseLeftButtonUp += new System.Windows.Input.MouseButtonEventHandler(this.refreshTextBlock_MouseLeftButtonUp);
            
            #line default
            #line hidden
            return;
            case 16:
            this.videoPanel = ((System.Windows.Controls.WrapPanel)(target));
            return;
            }
            this._contentLoaded = true;
        }
    }
}

