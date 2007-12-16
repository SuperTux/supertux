using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.Data;
using System.IO;
using PhysFS_NET;

namespace TestApp
{
	/// <summary>
	/// Summary description for Form1.
	/// </summary>
	public class TestAppForm : System.Windows.Forms.Form
	{
      private System.Windows.Forms.Label label2;
      private System.Windows.Forms.Button RefreshCDsButton;
      private System.Windows.Forms.ListBox CDDrivesList;
      private System.Windows.Forms.ListBox SearchPathList;
      private System.Windows.Forms.Label label1;
      private System.Windows.Forms.TextBox EnumFilesPath;
      private System.Windows.Forms.ListBox EnumList;
      private System.Windows.Forms.Label label3;
      private System.Windows.Forms.TextBox NewSearchPathText;
      private System.Windows.Forms.Button AddSearchPathButton;
      private System.Windows.Forms.Button RemovePathButton;
      private System.Windows.Forms.Button RefreshEnumList;
      private System.Windows.Forms.Button RefreshSearchPathButton;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public TestAppForm()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			//
			// TODO: Add any constructor code after InitializeComponent call
			//
		}

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
			if( disposing )
			{
				if (components != null) 
				{
					components.Dispose();
				}
			}
			base.Dispose( disposing );
		}

		#region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
         this.label2 = new System.Windows.Forms.Label();
         this.RefreshCDsButton = new System.Windows.Forms.Button();
         this.CDDrivesList = new System.Windows.Forms.ListBox();
         this.SearchPathList = new System.Windows.Forms.ListBox();
         this.label1 = new System.Windows.Forms.Label();
         this.EnumFilesPath = new System.Windows.Forms.TextBox();
         this.EnumList = new System.Windows.Forms.ListBox();
         this.label3 = new System.Windows.Forms.Label();
         this.RefreshEnumList = new System.Windows.Forms.Button();
         this.NewSearchPathText = new System.Windows.Forms.TextBox();
         this.AddSearchPathButton = new System.Windows.Forms.Button();
         this.RemovePathButton = new System.Windows.Forms.Button();
         this.RefreshSearchPathButton = new System.Windows.Forms.Button();
         this.SuspendLayout();
         // 
         // label2
         // 
         this.label2.Location = new System.Drawing.Point(8, 8);
         this.label2.Name = "label2";
         this.label2.Size = new System.Drawing.Size(136, 16);
         this.label2.TabIndex = 2;
         this.label2.Text = "Available CD-ROM Drives";
         // 
         // RefreshCDsButton
         // 
         this.RefreshCDsButton.Location = new System.Drawing.Point(8, 152);
         this.RefreshCDsButton.Name = "RefreshCDsButton";
         this.RefreshCDsButton.Size = new System.Drawing.Size(72, 24);
         this.RefreshCDsButton.TabIndex = 4;
         this.RefreshCDsButton.Text = "Refresh";
         this.RefreshCDsButton.Click += new System.EventHandler(this.RefreshCDsButton_Click);
         // 
         // CDDrivesList
         // 
         this.CDDrivesList.Location = new System.Drawing.Point(8, 24);
         this.CDDrivesList.Name = "CDDrivesList";
         this.CDDrivesList.Size = new System.Drawing.Size(136, 121);
         this.CDDrivesList.TabIndex = 7;
         // 
         // SearchPathList
         // 
         this.SearchPathList.Location = new System.Drawing.Point(152, 24);
         this.SearchPathList.Name = "SearchPathList";
         this.SearchPathList.Size = new System.Drawing.Size(248, 95);
         this.SearchPathList.TabIndex = 8;
         // 
         // label1
         // 
         this.label1.Location = new System.Drawing.Point(152, 8);
         this.label1.Name = "label1";
         this.label1.Size = new System.Drawing.Size(136, 16);
         this.label1.TabIndex = 10;
         this.label1.Text = "Search Path";
         // 
         // EnumFilesPath
         // 
         this.EnumFilesPath.Location = new System.Drawing.Point(408, 128);
         this.EnumFilesPath.Name = "EnumFilesPath";
         this.EnumFilesPath.Size = new System.Drawing.Size(208, 20);
         this.EnumFilesPath.TabIndex = 11;
         this.EnumFilesPath.Text = "";
         // 
         // EnumList
         // 
         this.EnumList.Location = new System.Drawing.Point(408, 24);
         this.EnumList.Name = "EnumList";
         this.EnumList.Size = new System.Drawing.Size(208, 95);
         this.EnumList.TabIndex = 12;
         // 
         // label3
         // 
         this.label3.Location = new System.Drawing.Point(408, 8);
         this.label3.Name = "label3";
         this.label3.Size = new System.Drawing.Size(136, 16);
         this.label3.TabIndex = 13;
         this.label3.Text = "Enumerate Files";
         // 
         // RefreshEnumList
         // 
         this.RefreshEnumList.Location = new System.Drawing.Point(544, 152);
         this.RefreshEnumList.Name = "RefreshEnumList";
         this.RefreshEnumList.Size = new System.Drawing.Size(72, 24);
         this.RefreshEnumList.TabIndex = 14;
         this.RefreshEnumList.Text = "Refresh";
         this.RefreshEnumList.Click += new System.EventHandler(this.RefreshEnumList_Click);
         // 
         // NewSearchPathText
         // 
         this.NewSearchPathText.Location = new System.Drawing.Point(152, 128);
         this.NewSearchPathText.Name = "NewSearchPathText";
         this.NewSearchPathText.Size = new System.Drawing.Size(248, 20);
         this.NewSearchPathText.TabIndex = 15;
         this.NewSearchPathText.Text = "";
         // 
         // AddSearchPathButton
         // 
         this.AddSearchPathButton.Location = new System.Drawing.Point(152, 152);
         this.AddSearchPathButton.Name = "AddSearchPathButton";
         this.AddSearchPathButton.Size = new System.Drawing.Size(72, 24);
         this.AddSearchPathButton.TabIndex = 9;
         this.AddSearchPathButton.Text = "Add Path";
         this.AddSearchPathButton.Click += new System.EventHandler(this.AddSearchPathButton_Click);
         // 
         // RemovePathButton
         // 
         this.RemovePathButton.Location = new System.Drawing.Point(232, 152);
         this.RemovePathButton.Name = "RemovePathButton";
         this.RemovePathButton.Size = new System.Drawing.Size(88, 24);
         this.RemovePathButton.TabIndex = 16;
         this.RemovePathButton.Text = "Remove Path";
         this.RemovePathButton.Click += new System.EventHandler(this.RemovePathButton_Click);
         // 
         // RefreshSearchPathButton
         // 
         this.RefreshSearchPathButton.Location = new System.Drawing.Point(328, 152);
         this.RefreshSearchPathButton.Name = "RefreshSearchPathButton";
         this.RefreshSearchPathButton.Size = new System.Drawing.Size(72, 24);
         this.RefreshSearchPathButton.TabIndex = 17;
         this.RefreshSearchPathButton.Text = "Refresh";
         this.RefreshSearchPathButton.Click += new System.EventHandler(this.RefreshSearchPathButton_Click);
         // 
         // TestAppForm
         // 
         this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
         this.ClientSize = new System.Drawing.Size(624, 309);
         this.Controls.AddRange(new System.Windows.Forms.Control[] {
                                                                      this.RefreshSearchPathButton,
                                                                      this.RemovePathButton,
                                                                      this.NewSearchPathText,
                                                                      this.RefreshEnumList,
                                                                      this.label3,
                                                                      this.EnumList,
                                                                      this.EnumFilesPath,
                                                                      this.label1,
                                                                      this.SearchPathList,
                                                                      this.CDDrivesList,
                                                                      this.RefreshCDsButton,
                                                                      this.label2,
                                                                      this.AddSearchPathButton});
         this.Name = "TestAppForm";
         this.Text = "PhysFS Test Application";
         this.Load += new System.EventHandler(this.TestAppForm_Load);
         this.ResumeLayout(false);

      }
		#endregion

		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main() 
		{
         Application.Run(new TestAppForm());
		}

      private void TestAppForm_Load(object sender, System.EventArgs e)
      {
      
      }

      private void RefreshCDsButton_Click(object sender, System.EventArgs e)
      {
         // Clear ths listbox if it contains any items
         CDDrivesList.Items.Clear();
         // Add the items to the list
         CDDrivesList.Items.AddRange(PhysFS.GetCDROMDrives());
      }

      private void RefreshSearchPathButton_Click(object sender, System.EventArgs e)
      {
         // Clear ths listbox if it contains any items
         SearchPathList.Items.Clear();
         // Add the items to the list
         SearchPathList.Items.AddRange(PhysFS.GetSearchPath());
      }

      private void AddSearchPathButton_Click(object sender, System.EventArgs e)
      {
         // Add search path
         PhysFS.AddToSearchPath(NewSearchPathText.Text, false);
         // Clear ths listbox if it contains any items
         SearchPathList.Items.Clear();
         // Add the items to the list
         SearchPathList.Items.AddRange(PhysFS.GetSearchPath());
      }

      private void RemovePathButton_Click(object sender, System.EventArgs e)
      {
         if(SearchPathList.SelectedItem != null)
         {
            PhysFS.RemoveFromSearchPath(SearchPathList.SelectedItem.ToString());
            // Clear ths listbox if it contains any items
            SearchPathList.Items.Clear();
            // Add the items to the list
            SearchPathList.Items.AddRange(PhysFS.GetSearchPath());
         }
      }

      private void RefreshEnumList_Click(object sender, System.EventArgs e)
      {
         EnumList.Items.Clear();
         EnumList.Items.AddRange(PhysFS.EnumerateFiles(EnumFilesPath.Text));
      }
	}
}
