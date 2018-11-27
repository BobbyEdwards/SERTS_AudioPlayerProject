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
        Me.SuspendLayout()
        '
        'Show_Files
        '
        Me.Show_Files.Location = New System.Drawing.Point(13, 13)
        Me.Show_Files.Name = "Show_Files"
        Me.Show_Files.Size = New System.Drawing.Size(75, 23)
        Me.Show_Files.TabIndex = 0
        Me.Show_Files.Text = "Refresh List"
        Me.Show_Files.UseVisualStyleBackColor = True
        '
        'File_List
        '
        Me.File_List.FormattingEnabled = True
        Me.File_List.Location = New System.Drawing.Point(13, 43)
        Me.File_List.Name = "File_List"
        Me.File_List.Size = New System.Drawing.Size(156, 134)
        Me.File_List.Sorted = True
        Me.File_List.TabIndex = 1
        '
        'SerialPort1
        '
        Me.SerialPort1.PortName = "COM4"
        '
        'Select_File_Button
        '
        Me.Select_File_Button.Location = New System.Drawing.Point(94, 14)
        Me.Select_File_Button.Name = "Select_File_Button"
        Me.Select_File_Button.Size = New System.Drawing.Size(75, 23)
        Me.Select_File_Button.TabIndex = 2
        Me.Select_File_Button.Text = "Select File"
        Me.Select_File_Button.UseVisualStyleBackColor = True
        '
        'Pause_Button
        '
        Me.Pause_Button.Location = New System.Drawing.Point(13, 184)
        Me.Pause_Button.Name = "Pause_Button"
        Me.Pause_Button.Size = New System.Drawing.Size(75, 23)
        Me.Pause_Button.TabIndex = 3
        Me.Pause_Button.Text = "Pause"
        Me.Pause_Button.UseVisualStyleBackColor = True
        '
        'Play_Button
        '
        Me.Play_Button.Location = New System.Drawing.Point(95, 183)
        Me.Play_Button.Name = "Play_Button"
        Me.Play_Button.Size = New System.Drawing.Size(75, 23)
        Me.Play_Button.TabIndex = 4
        Me.Play_Button.Text = "Play"
        Me.Play_Button.UseVisualStyleBackColor = True
        '
        'Form1
        '
        Me.AutoScaleDimensions = New System.Drawing.SizeF(6.0!, 13.0!)
        Me.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font
        Me.ClientSize = New System.Drawing.Size(284, 261)
        Me.Controls.Add(Me.Play_Button)
        Me.Controls.Add(Me.Pause_Button)
        Me.Controls.Add(Me.Select_File_Button)
        Me.Controls.Add(Me.File_List)
        Me.Controls.Add(Me.Show_Files)
        Me.Name = "Form1"
        Me.Text = "Form1"
        Me.ResumeLayout(False)

    End Sub
    Friend WithEvents Show_Files As System.Windows.Forms.Button
    Friend WithEvents File_List As System.Windows.Forms.ListBox
    Friend WithEvents SerialPort1 As System.IO.Ports.SerialPort
    Friend WithEvents Select_File_Button As System.Windows.Forms.Button
    Friend WithEvents Pause_Button As System.Windows.Forms.Button
    Friend WithEvents Play_Button As System.Windows.Forms.Button

End Class
