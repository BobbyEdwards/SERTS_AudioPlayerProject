<Global.Microsoft.VisualBasic.CompilerServices.DesignerGenerated()> _
Partial Class Form1
    Inherits System.Windows.Forms.Form

    'Form overrides dispose to clean up the component list.
    <System.Diagnostics.DebuggerNonUserCode()> _
    Protected Overrides Sub Dispose(ByVal disposing As Boolean)
        Try
            If disposing AndAlso components IsNot Nothing Then
                components.Dispose()
            End If
        Finally
            MyBase.Dispose(disposing)
        End Try
    End Sub

    'Required by the Windows Form Designer
    Private components As System.ComponentModel.IContainer

    'NOTE: The following procedure is required by the Windows Form Designer
    'It can be modified using the Windows Form Designer.  
    'Do not modify it using the code editor.
    <System.Diagnostics.DebuggerStepThrough()> _
    Private Sub InitializeComponent()
        Me.components = New System.ComponentModel.Container()
        Me.Show_Files = New System.Windows.Forms.Button()
        Me.File_List = New System.Windows.Forms.ListBox()
        Me.SerialPort1 = New System.IO.Ports.SerialPort(Me.components)
        Me.Select_File_Button = New System.Windows.Forms.Button()
        Me.Pause_Button = New System.Windows.Forms.Button()
        Me.Play_Button = New System.Windows.Forms.Button()
        Me.Next_Button = New System.Windows.Forms.Button()
        Me.Previous_Button = New System.Windows.Forms.Button()
        Me.Current_Song_Label = New System.Windows.Forms.Label()
        Me.SuspendLayout()
        '
        'Show_Files
        '
        Me.Show_Files.Location = New System.Drawing.Point(90, 37)
        Me.Show_Files.Name = "Show_Files"
        Me.Show_Files.Size = New System.Drawing.Size(75, 23)
        Me.Show_Files.TabIndex = 0
        Me.Show_Files.Text = "Refresh List"
        Me.Show_Files.UseVisualStyleBackColor = True
        '
        'File_List
        '
        Me.File_List.FormattingEnabled = True
        Me.File_List.Location = New System.Drawing.Point(90, 66)
        Me.File_List.Name = "File_List"
        Me.File_List.Size = New System.Drawing.Size(271, 134)
        Me.File_List.Sorted = True
        Me.File_List.TabIndex = 1
        '
        'SerialPort1
        '
        Me.SerialPort1.PortName = "COM5"
        '
        'Select_File_Button
        '
        Me.Select_File_Button.Location = New System.Drawing.Point(286, 37)
        Me.Select_File_Button.Name = "Select_File_Button"
        Me.Select_File_Button.Size = New System.Drawing.Size(75, 23)
        Me.Select_File_Button.TabIndex = 2
        Me.Select_File_Button.Text = "Select File"
        Me.Select_File_Button.UseVisualStyleBackColor = True
        '
        'Pause_Button
        '
        Me.Pause_Button.Location = New System.Drawing.Point(143, 245)
        Me.Pause_Button.Name = "Pause_Button"
        Me.Pause_Button.Size = New System.Drawing.Size(75, 23)
        Me.Pause_Button.TabIndex = 3
        Me.Pause_Button.Text = "Pause"
        Me.Pause_Button.UseVisualStyleBackColor = True
        '
        'Play_Button
        '
        Me.Play_Button.Location = New System.Drawing.Point(231, 246)
        Me.Play_Button.Name = "Play_Button"
        Me.Play_Button.Size = New System.Drawing.Size(75, 23)
        Me.Play_Button.TabIndex = 4
        Me.Play_Button.Text = "Play"
        Me.Play_Button.UseVisualStyleBackColor = True
        '
        'Next_Button
        '
        Me.Next_Button.Location = New System.Drawing.Point(286, 280)
        Me.Next_Button.Name = "Next_Button"
        Me.Next_Button.Size = New System.Drawing.Size(75, 23)
        Me.Next_Button.TabIndex = 5
        Me.Next_Button.Text = "Next"
        Me.Next_Button.UseVisualStyleBackColor = True
        '
        'Previous_Button
        '
        Me.Previous_Button.Location = New System.Drawing.Point(90, 280)
        Me.Previous_Button.Name = "Previous_Button"
        Me.Previous_Button.Size = New System.Drawing.Size(75, 23)
        Me.Previous_Button.TabIndex = 6
        Me.Previous_Button.Text = "Previous"
        Me.Previous_Button.UseVisualStyleBackColor = True
        '
        'Current_Song_Label
        '
        Me.Current_Song_Label.AutoSize = True
        Me.Current_Song_Label.Location = New System.Drawing.Point(87, 216)
        Me.Current_Song_Label.Name = "Current_Song_Label"
        Me.Current_Song_Label.Size = New System.Drawing.Size(72, 13)
        Me.Current_Song_Label.TabIndex = 7
        Me.Current_Song_Label.Text = "Current Song:"
        '
        'Form1
        '
        Me.AutoScaleDimensions = New System.Drawing.SizeF(6.0!, 13.0!)
        Me.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font
        Me.ClientSize = New System.Drawing.Size(475, 365)
        Me.Controls.Add(Me.Current_Song_Label)
        Me.Controls.Add(Me.Previous_Button)
        Me.Controls.Add(Me.Next_Button)
        Me.Controls.Add(Me.Play_Button)
        Me.Controls.Add(Me.Pause_Button)
        Me.Controls.Add(Me.Select_File_Button)
        Me.Controls.Add(Me.File_List)
        Me.Controls.Add(Me.Show_Files)
        Me.Name = "Form1"
        Me.Text = "Audio File Player"
        Me.ResumeLayout(False)
        Me.PerformLayout()

    End Sub
    Friend WithEvents Show_Files As System.Windows.Forms.Button
    Friend WithEvents File_List As System.Windows.Forms.ListBox
    Friend WithEvents SerialPort1 As System.IO.Ports.SerialPort
    Friend WithEvents Select_File_Button As System.Windows.Forms.Button
    Friend WithEvents Pause_Button As System.Windows.Forms.Button
    Friend WithEvents Play_Button As System.Windows.Forms.Button
    Friend WithEvents Next_Button As Button
    Friend WithEvents Previous_Button As Button
    Friend WithEvents Current_Song_Label As Label
End Class
